/**************************************************************************
文件名称：	App_InitMode.c
说    明：	初始化模式（应用层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
备    注:
修订记录：
**************************************************************************/
#include "Include.h"

eInitModeTask eInitTask_Sta;

void App_InitMode(void)
{
	switch( eInitTask_Sta )
	{
		//初始化设置
		case Init_Set:
			Drv_Adc_Init();	//AD初始化
			Drv_Adc_Channel_Set(ACM_ACM);
			Delay1ms(5);

			#if Func_debug
				Drv_UartTX_Init();
			#endif

			//默认切换到下一状态
			eInitTask_Sta = Init_Disp;

			//EEPROM参数检查(检查失败则重置为出厂默认数据，检查成功则继续运行)
			//EEPROM参数检查(Er5错误处理，将参数重置为默认值)
			Parm_AutoCheck();
			break;

		//全屏显示，获取ADC实际500ms值
		case Init_Disp:
			//低电压检测
			LBD_Chk();
			if( uStaFlag.bits.LowBat )
			{
				Clr_Disp();
				Disp_LowBat();	//显示低电压信息
				Auto_TurnOff_Time_Sel();	//关机时间选择
				eInitTask_Sta = Init_Err;
				#if Have_Voice_Func
					if(  uSetFlag.bits.Voiceflag ==1 )
					{
						PlayStatueParam(2 , 0 , 0);
						PlayStatueParam(1 , Play_Lowbat,0);
					}
				#endif
			}
			else
			{
				Disp_All();
				F_LED_Enable = Enable;	//LED开启
				Auto_TurnOff_Time_Sel();	//关机时间选择
                
				//默认切换到下一状态
				eInitTask_Sta = Init_ADDoff;
			}
			break;

		//检测Adc的Doff值
		case Init_ADDoff:
			//Adc Doff检测14次*0.016ms=224ms(取14次去4次最大最小，取8次平均)，实际300ms
			if( Get_Adc_Avg() )
			{
				g_AdcDoff = g_AdcSum;
				Drv_PGA_Init(Adc_PGA_Gain);
				Drv_Adc_Channel_Set(ACM_ACM);
				eInitTask_Sta = Init_OpDoff;
			}
			break;

		//检测PGA的Doff值
		case Init_OpDoff:
			//Adc offset检测14次*0.016ms=224ms(取14次去4次最大最小，取8次平均)，实际300ms
			if( Get_Adc_Avg() )
			{
				g_OpDoff = g_AdcSum;
				// Drv_Adc_Offset_Set( Offset_25 );
				// eInitTask_Sta = Init_OpOffsetDoff;
				Adc_Channel_Init(TPTONTC);	//ADC初始化通道切换
				eInitTask_Sta = Init_Ntc;
			}
			break;

		//检测thermistor温度
		case Init_Ntc:
			//检测ntc确保稳定再显示224*2=448ms，实际700ms左右
			if( Get_Ntc_Count() )
			{
				Ntc_Caculate();
				NtcTable_Check();
				if( !uErrFlag.bits.Er2 )	//这里不处理Er2
				{
					NtcTable_Find();
				}
				eInitTask_Sta = Init_Wait;
			}
			break;

		//错误处理
		case Init_Err:
			//持续显示低电LO信息（每次进入都重新写一次，防止被其他模块清掉）
			Disp_LowBat();
			//等待测量键释放后再次短按才进入睡眠（防止长按测量直接关机）
			if( uKeyPress.bits.TKeyPress && uKeyRelease.bits.TKeyRelease )
			{
				uKeyRelease.bits.TKeyRelease=0;
				eInitTask_Sta = Init_Set;	//将当前状态设置为初始状态
				eMain_Task = Task_Sleepmode;
			}
			if(!uKeyPress.bits.TKeyPress)
			{
				uKeyRelease.bits.TKeyRelease=1;
			}
			break;

		//1.5s全屏显示等待
		case Init_Wait:
			if( !F_LED_Enable )
				eInitTask_Sta = Init_Key;
			break;

		//按键检测
		case Init_Key:
	 		if( uKeyPress.bits.MKeyPress&&eTestmode_num== Earmode)
			{
				uKeyPress.bits.MKeyPress=0;
		       	eTestmode_num=Foreheadmode;
			   	Adc_Channel_Init(TPTONTC);
			  	uStaFlag.bits.ProbeCover = 1;
			}
			//是否按下3s进入全屏自检模式
			if( uKeyPress.bits.TKeyPress )
			{
				HalKey_Set_KeyMode(Func_Short_Long, &sSetKey);	//设置键为短长按
				//检测设置键是否长按进入自检模式
				if( uKeyHold.bits.SKeyHold &&  uKeyRelease.bits.SKeyRelease )
				//if( uKeyHold.bits.SKeyHold  )
				{
					Clr_Disp();
					Disp_BadFace();
					Clr_All_Memory();
					uSetFlag.bits.Unit = Unit_C;	//设置显示默认C
					Adc_Channel_Init(TPTONTC);		//切换到NTC通道(AI2_ACM)，否则读TP通道导致Er2  
					NtcTableWider_Check();
					eTestmode_num = Insptectmode;	//进入自检模式切换当前模式为检测
				}
				//检测等待测试/测量键抬起后退出检测，进入ready状态或关机
				if( uKeyRelease.bits.TKeyRelease )
				{
					HalKey_Set_KeyMode(Func_Long, &sSetKey);	//设置键为长按
					eInitTask_Sta = Init_End;
				}
			}
			else
			{
				HalKey_Set_KeyMode(Func_Long, &sSetKey);	//设置键为长按
				eInitTask_Sta = Init_End;
			}
			break;

		//初始化结束，将当前状态设为初始状态
		case Init_End:
			Clr_Disp();
			Disp_FullBat();
			HalKey_KeyClr();	//清除所有按键信息
			eInitTask_Sta = Init_Set;		//下次进入将当前状态设置为初始状态
			eMain_Task = Task_ReadyMode;	//进入第一个任务
			eReadyTask_Sta = Ready_Init;
			break;

		//默认情况
		default:
			break;
	}
}

//初始化模式，在关机时设置时间、日期、单位等参数。类似睡眠模式下的Sleep_false函数
void Set_Reset(void)
{
	//设置默认参数，初始化时间日期
	g_Hour = 0;
	g_Minute = 0;
	g_Day = 1;
	g_Month = 1;
	g_Year = Default_Year;
	g_Second = 0;
	uSetFlag.bits.TimeFormat = TimeFormat_24H;	//默认24小时制
	uSetFlag.bits.Unit = Unit_C;	//默认C单位
	uSetFlag.bits.Unit_Change = Unit_Change_En;	//默认单位可切换
	uSetFlag.bits.Motorflag = Disable;	//默认关闭震动
	uSetFlag.bits.Voiceflag = Enable;	//默认开启语音
}