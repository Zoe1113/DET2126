/**************************************************************************
文件名称：	App_ReadyMode.c
说    明：	待机前等待测试模式（应用层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
备    注:
修订记录：
**************************************************************************/
#include "Include.h"

eReadyModeTask eReadyTask_Sta;
uint8 timeout_ready=0;
uint8 first_InsReady=0;

void App_ReadyMode(void)
{
	App_PCKeyProcess();
	TKeyProcess();
	SkeyProcess();
	MemKeyProcess();
	Disp_Unit();	//显示单位

	Cal_Inspect_Detect();//校准检测
	switch( eReadyTask_Sta )
	{
		//初始化
		case Ready_Init:
			#if !Secondary_voltage
				lcd_bat_clr();
			#endif
			Disp_VoiceSign();
			//g_50ms_Count = DispTime_Init;	//循环显示时间、日期、ntc、环境温度值
			HalKey_KeyClr();	//清除所有按键信息
			uSetFlag.bits.Ready_First = 1;
			if(eTestmode_num == Insptectmode)
			{
				LED_CloseAll();
				LED_Green_En();		//生产模式首次进入，绿色背光15s
				g_15s_Count = CountDown_15s;
			}
			eReadyTask_Sta = Ready_Refresh;
			break;

		//5秒超时判断
		case Ready_Timeout:
			if(eTestmode_num == Earmode || eTestmode_num == Blackbodymode || eTestmode_num ==Insptectmode )
			{
				if(Time_CountDown_5s_timeout(RUN))//超时启动
				{
					if(uErrFlag.bits.Lo || uErrFlag.bits.Hi||uErrFlag.bits.Er3)
					{
						eReadyTask_Sta = Ready_Refresh;      //等待时间到，需要刷新显示
					}
					else	
					{
						eReadyTask_Sta = Ready_NoRefresh;      //等待时间到，不需要刷新显示
					}
				}

			}
			else
			{
				eReadyTask_Sta = Ready_NoRefresh;
			}

			//如果发生Er2错误，切换到Er2处理状态
			if(uErrFlag.bits.Er2)
            {
                eReadyTask_Sta = Ready_DisEr2;
            }
		
			break;

		//等待状态（需要刷新显示）
		case Ready_Refresh:
            Clr_Disp888();
			Disp_Ready();	//待机画面显示_ _._
			Disp_ModeSign();//模式标志
			//g_50ms_Count = DispTime_Init;
	
			if(eTestmode_num != Insptectmode)
			{
				LED_CloseAll();
				LED_Green_En();		//绿色背光
				g_15s_Count = CountDown_15s;	//记忆显示15秒倒计时
			}
			else if(eTestmode_num == Insptectmode&&first_InsReady==0)
			{
				first_InsReady=1;
				LED_CloseAll();
				LED_Green_En();		//绿色背光
				g_15s_Count = CountDown_15s;	//记忆显示15秒倒计时
			}
			
            eReadyTask_Sta = Ready_WaitReady;
            
            break;

		//等待状态（不需要刷新显示）
		case Ready_NoRefresh:
			Disp_ModeSign();//模式标志
            eReadyTask_Sta = Ready_WaitReady;
			break;
        //等待就绪状态，不显示
        case Ready_WaitReady:
			// 显示错误信息时，需要保持错误显示
			uErrFlag.g_ErrFlag = 0;
			F_Mem_FirstEnter = 0; //清除记忆
            Auto_TurnOff_Time_Sel();//自动关机
			if(eTestmode_num != Objectmode && eTestmode_num != Foreheadmode && eTestmode_num != Insptectmode)
			{
				LED_CloseAll();
				LED_Green_En();		//绿色背光
				g_15s_Count = CountDown_15s;	//记忆显示15秒倒计时
			}

			// 播放就绪提示
			if(uSetFlag.bits.Ready_First )
			{
				uSetFlag.bits.Ready_First = 0;  // 清除首次进入标志
				
				// 语音提示
				if(uSetFlag.bits.Voiceflag == 1)
				{
					#if Have_Voice_Func
						PlayStatueParam(2, 0, 0);
						PlayStatueParam(1, Play_Clean, 0);
					#else
						BZ_Beep230();
						BZ_Beep230();		//蜂鸣2声
					#endif
				}
				
				// 震动提示
				if(uSetFlag.bits.Motorflag == 1)
				{
					MT_Vib230();
					MT_Vib230();
				}
			}
			//耳温模式下tp通道检测
            if (eTestmode_num == Earmode)
            {
                Adc_Channel_Init(NTCTOTP);
            }
            eReadyTask_Sta = Ready_ReadyOk;		//准备完成，状态切换到Ready_ReadyOk
            break;
		case Ready_DisEr1:
		{
			static uint8 Er1_Nb_InitDone = 0;
			if(!Er1_Nb_InitDone)
			{
				// 初始化 Er1 显示 + 非阻塞动画
				uErrFlag.g_ErrFlag = 0;
				uErrFlag.bits.Er1 = 1;
				Disp_FourSecLoop_Init();
				Er1_Nb_InitDone = 1;
			}

			if(Disp_FourSecLoop_Step())   // 返回 1 = 动画完成（5秒结束）
			{
				LED_CloseAll();
				LED_Green_En();		//绿色背光
				g_15s_Count = CountDown_15s;	//记忆显示15秒倒计时
				Time_CountDown_5s_timeout(RESET);
				uSetFlag.bits.Ready_First = 1;
				uErrFlag.bits.Er1 = 0;
				eReadyTask_Sta = Ready_Refresh;
				eMain_Task = Task_ReadyMode;
				Er1_Nb_InitDone = 0;
			}
			break;
		}

		case Ready_DisEr2:
			
			Er2_Display_Sound(RUN);		//Er2错误处理
			break;

		case Ready_DisEr6:
			//Er6_Display_Sound(RUN);		//Er6错误处理
			break;
		//就绪状态
		case Ready_ReadyOk:
			Disp_ModeSign();//模式标志
			uStaFlag.bits.Fever = 0;	//发烧标志位，测试完成后清零
			break;

		default:
			break;
	}

}

/**
 * er2错误处理;	
 * cmd:	为Enable时处理错误
 * 		为Disable时在错误处理完成时清除标志位
 * 
*/
void Er2_Display_Sound(bit cmd)
{
	static uint8 Er2_First_Enter = 0;      //注意标志位要在关机时清零，否则在Er2时关机后再次第一次进入Er2时没有处理
	if(cmd == RESET)//er2错误处理完成，清除标志位
	{
		Er2_First_Enter = 0;
		goto END;                         //使用跳转语句使函数执行到末尾结束，避免重复代码
	}
	if(Er2_First_Enter == 0)
	{
		uSetFlag.bits.Ready_First =1;
		Auto_TurnOff_Time_Sel();
		Time_CountDown_5s_timeout(RESET);		//超时计时关闭标志位
		
		LED_CloseAll();		//关闭LED
		Disp_ErrMsg();		//显示错误信息
		Disp_ModeSign();//模式标志
		lcd_mem_clr();
		if( uSetFlag.bits.Voiceflag == 1 )
		{
			#if Have_Voice_Func	//语音功能处理
				g_DiDo = uErrFlag.g_ErrFlag;
				PlayStatueParam(2 , 0 , 0);
				PlayStatueParam(1 , Play_Stop,0);
				PlayStatueParam(1 , Play_Errmsg,0);
			#else
			    BZ_Beep230();
				BZ_Beep230();
				BZ_Beep230();
				BZ_Beep230();
			#endif
		} 
		if( uSetFlag.bits.Motorflag == 1 )
		{
			MT_Vib230();			
		}
		Er2_First_Enter = 1;
		uErrFlag.bits.Er1 = 0;
	}
	else
	{
		if(uErrFlag.bits.Er2 == 0)
		{
			Er2_First_Enter = 0;
			eReadyTask_Sta = Ready_Refresh;
			eMain_Task = Task_ReadyMode;
		}

	}
END: ;        //跳转来到这里，函数结束

}

/**************************************************************************
函数名称：	void Er3_Display_Sound(bit cmd)
函数功能：	Er3错误的报警处理（仅蜂鸣+震动，无背光，仅首次进入时报警一次）
输入参数：	cmd->   0：关闭错误报警   1：开启Er3报警
输出参数：	无
返回值  ：	无
**************************************************************************/
void Er3_Display_Sound(bit cmd)
{
	static uint8 Er3_First_Enter = 0;
	if(cmd == RESET)
	{
		Er3_First_Enter = 0;
		return;
	}
	if(Er3_First_Enter == 0)
	{
		Er3_First_Enter = 1;
		
		if( uSetFlag.bits.Voiceflag == 1 )
		{
		    BZ_Beep160();
			BZ_Beep160();
			BZ_Beep160();
			BZ_Beep160();
		} 
		if( uSetFlag.bits.Motorflag == 1 )
		{
			MT_Vib160();	
			MT_Vib160();
			MT_Vib160();	
			MT_Vib160();		
		}
	}
}

/**************************************************************************
函数名称：	void Er5_Display_Sound(bit cmd)
函数功能：	Er5错误的报警处理（仅蜂鸣+震动，无背光，仅首次进入时报警一次）
输入参数：	cmd->   0：关闭错误报警   1：开启Er5报警
输出参数：	无
返回值  ：	无
**************************************************************************/
void Er5_Display_Sound(bit cmd)
{
	static uint8 Er5_First_Enter = 0;
	if(cmd == RESET)
	{
		Er5_First_Enter = 0;
		return;
	}
	if(Er5_First_Enter == 0)
	{
		Er5_First_Enter = 1;
		if( uSetFlag.bits.Voiceflag == 1 )
		{
		    BZ_Beep160();
			BZ_Beep160();
			BZ_Beep160();
			BZ_Beep160();
		} 
		if( uSetFlag.bits.Motorflag == 1 )
		{
			MT_Vib160();	
			MT_Vib160();
			MT_Vib160();	
			MT_Vib160();		
		}
	}
}


