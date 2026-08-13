#include "Include.h"

//主程序
void main(void)
{
	WDTR = 0x5A;	//喂狗
	//系统时钟设定
    OSCM = 0x00;		//开启内置IHRC=8M，Fcpu=Normal mode
	Delay50us(100);
	//上电的系统时间初始化
	//部分参数设置
	
	uSetFlag.bits.Unit = Unit_C;	//默认C单位
	uSetFlag.bits.Unit_Change = Unit_Change_En;	//默认单位可切换

	uSetFlag.bits.Voiceflag = Enable;	//默认开启语音
	uSetFlag.bits.Motorflag=Disable;	//默认关闭电机;

	GPIO_Init();//IO口设置
	Lcd_Init();	//Lcd设置
	CF_Check();	//单位状态确认
	HalKey_Set_KeyMode(Func_Short, &sMemKey);		//记忆键设为长按
	HalKey_Set_KeyMode(Func_Long, &sTestKey);	//测量键设为长按
	HalKey_Set_KeyMode(Func_Short, &sModeKey);	//耳套键为短按
	HalKey_Set_KeyMode(Func_Long, &sSetKey);	//设置键为长按
	eTestmode_num = Earmode;	//默认耳温模式
	//低电压检测
	LBD_Chk();
	//Clr_All_Memory();
	//Param_Init();
	//Disp_PAS();
	//Delay10ms(100);
	if( uStaFlag.bits.LowBat )
	{
		Clr_Disp();
		Disp_LowBat();
		Auto_TurnOff_Time_Sel();	//关机时间选择
		uKeyRelease.bits.TKeyRelease = 0;
		eMain_Task = Task_InitMode;
		eInitTask_Sta = Init_Err;
	}
	else
	{
		#if Power_0FF
			eMain_Task = Task_Sleepmode;
			eSleepTask_Sta = Sleep_true;
		#elif ON_Set
			eMain_Task = Task_Setmode;
			eSleepTask_Sta = Sleep_true;
		#else
			eMain_Task = Task_Sleepmode;
			eSleepTask_Sta = Sleep_End;
		#endif
	}

	Parm_AutoCheck();
	
	TC1Init();	//TC1设置（10ms）
	//BuzzerTCInit(250,50);
	FGIE = 1;	//使能总中断等
	Cal_Inspect_Detect();//厂商进入绑定

	//while(1)
	//{
	//	WDTR = 0x5A;	//喂狗
	//	lcd5 = 0x03;
    //    lcd6 = 0x0E;
    //    lcd3 = 0x0B;
    //   lcd4 = 0x0C;
    //   lcd1 = 0x08;
    //    lcd2 = 0x0E;
	//}	


	while((!Port_Mem||!Port_Test||!Port_Set)&&!uStaFlag.bits.LowBat)
	{
		WDTR = 0x5A;	//喂狗
	}
	while(1)
	{
		WDTR = 0x5A;	//喂狗
		//10ms基本定时器
		if(F_10ms)
		{
			F_10ms=0;
			Time_Creat_20ms_50ms();
			//NTC采集
			if (eMain_Task == Task_Memorymode || eMain_Task == Task_ReadyMode)
			{
    			ReadyMode_NtcMeas();            //F_ReadyOk置位则采集NTC
				//只在Ready_ReadyOk状态下才触发Er2检测（避免年龄分段切换时触发）
				if(uErrFlag.bits.Er2 && eReadyTask_Sta > 0)
				{
					eReadyTask_Sta = Ready_DisEr2;
					eMain_Task = Task_ReadyMode;
				}
			}
		}
		//20ms任务轮询
		if(F_20ms)
		{
			F_20ms = 0;

			//扫描按键状态
			HalKey_KeyScan();

			//三色背光
			if( F_LED_Enable )
				Light_RGB();

			if( (eMain_Task == Task_ReadyMode )||( eMain_Task == Task_Memorymode ))
			{
				//15s背光倒计时
				Led_CountDown_15s();

			}

			//自动关机
			if (eMain_Task == Task_Memorymode ||eMain_Task == Task_ReadyMode || eMain_Task == Task_Setmode || eMain_Task == Task_InitMode || eMain_Task == Task_Unitmode  || eMain_Task == Task_ParamModifymode  )
				Auto_TurnOff();
		}
		//500ms更新显示
 		if(F_500ms)
		{
			F_500ms = 0;

			#if Secondary_voltage
				//低电压闪烁
				if( eMain_Task==Task_InitMode|| eMain_Task == Task_BondTestmode )//避免全显时闪烁
				{
				}
				else if( eMain_Task==Task_Setmode || eMain_Task==Task_Sleepmode|| eMain_Task==Task_Calimode|| eMain_Task ==  Task_Unitmode )//设置和关机，校准时不显示
				{
					lcd_bat_clr();
					lcd_fullbat_clr();
				}
				else
				{
					LVD_Display();
				}
			#endif
		}

		#if Have_Voice_Func
			if( uSetFlag.bits.Voiceflag )
				App_PlayVoice3();
		#endif

		//主任务(10ms轮询，因为ADC基本是16ms，不可以20ms）
		if( F_10ms_task )
		{
			F_10ms_task = 0;

			switch(eMain_Task)
			{
				//开机初始化状态（即全显前的流程）
				case Task_InitMode:
					App_InitMode();
					break;

				//全显后测量前这一阶段等待就绪状态
				case Task_ReadyMode:
					App_ReadyMode();
					break;

				//测量模式任务（里面细分耳温、额温、物温、生产、黑体）
				case Task_Testingmode:
					App_TestingMode();
					break;

				//单位切换模式任务
				case Task_Unitmode:
					App_SetMode();
					break;

				//设置模式任务
				case Task_Setmode:
					App_SetMode();
					break;

				//校准模式任务
				case Task_Calimode:
					App_CaliMode();
					break;

				//距离校准模式任务
				#if Distence_En
				case Task_Discali:
					App_Discali();
					break;
				#endif 

				//绑定模式任务
				case Task_BondTestmode:
					App_BondTestMode();
					break;


				//参数调整模式任务
				#if ParamModif
				case Task_ParamModifymode:
					App_SetMode();
					break;
				#endif

				//关机模式任务
				case Task_Sleepmode:
					App_Sleep();
					break;

				//记忆模式任务
				case Task_Memorymode:
					App_Memory();
					break;

				//标配保留
				default:
					break;
			}
		}
	}
}
