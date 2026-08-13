/**************************************************************************
文件名称：	App_Sleep.c
说    明：	关机模式（应用层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
修订记录：
**************************************************************************/
#include "Include.h"

eSleep_Status eSleepTask_Sta;

void App_Sleep(void)
{
	static uint8 l_buf;
	switch ( eSleepTask_Sta )
	{
		case Sleep_false:
			if( eTestmode_num == Insptectmode)
			{
				eTestmode_num = Earmode;
				Set_Reset();
				CF_Check();
				Clr_All_Memory();
			}
			if(eTestmode_num == Blackbodymode )
			{
				eTestmode_num = Earmode;
			}
			uErrFlag.g_ErrFlag = 0;

			Time_CountDown_5s_timeout(RESET);
			
			Er2_Display_Sound(RESET);
			Er3_Display_Sound(RESET);
			eReadyTask_Sta = Ready_Init;
			eInitTask_Sta = Init_Set;
			F_Mem_FirstEnter = 0;
			eBeep_Status = Stop_beep;
			Drv_BZ_Disable();
			LED_CloseAll();
			Disp_OFF();
			Delay10ms(50);
			Clr_Disp();
			eSleepTask_Sta = Sleep_waitkey;
			break;

		case Sleep_waitkey:
			if ( uKeyPress.bits.TKeyPress && !uStaFlag.bits.LowBat)
			{
				if( uKeyHold.bits.TKeyHold )
				{
					lcd_point_en();
					if( uSetFlag.bits.Voiceflag )
					{
						BZ_Beep400();
						BZ_Beep400();
						BZ_Beep400();
					}
					while( !Port_Test )
					{
						WDTR = 0X5A;
					}
					Disp_CAL();
					Delay10ms(100);
					Disp_Code(Soft_Code);
					Delay10ms(100);
					Disp_Version(Soft_External_Version);
					Delay10ms(100);
					#if Secondary_voltage
						LVD_Display();
					#else
						Disp_FullBat();
					#endif
					Adc_Channel_Init(TPTONTC);
					Er2_Display_Sound(RESET);
					Er3_Display_Sound(RESET);
					uKeyPress.bits.TKeyPress = 0;
					eTestmode_num = Blackbodymode;
					eSleepTask_Sta = Sleep_false;
					eMain_Task = Task_ReadyMode;
					eReadyTask_Sta = Ready_Init;
				}
			}
			else 
			{      
				eSleepTask_Sta = Sleep_true;
				uStaFlag.g_StatusFlag &= 0x02;
			}
			break;

		//完全关机状态
	case Sleep_true:
			Drv_UartTX_Disable();	//关UART TX
			Drv_UartRX_Disable();	//关UART RX
			I2C_Disable();		//关I2C
			GPIO_PowerDown();	//IO口省电设置
			FCLKMD = 1;			//切到Slow mode
			NOP(2);
			FSTPHX = 1;			//关IHRC
			NOP(2);
			FDA1EN = 0;			//关DAC1
			FDA2EN = 0;			//关DAC2
			FOPA1EN = 0;		//关OP1
			FOPA2EN = 0;		//关OP2
			FAMPEN = 0 ;		//关PGA
			FPCHPEN = 0 ;		//关PGA chopper
			FACHPEN = 0 ;		//关ADC chopper
			FADC1EN = 0 ;		//关ADC1
			FADC2EN = 0;		//关ADC2
			FAVEN = 0 ;			//关AVE电压
			FACMEN = 0 ;		//关ACM电压
			FAVDDREN = 0 ;		//关avddr电压
			FBGCHP = 0;			//关bandgap chooper
			FBGREN = 0 ;		//关bandgap电压
			FLCDBNK = 1 ;		//All of the LCD dots off
			FLCDEN = 0 ;		//关LCD
			FLCDMOD0 = 1 ;
			FLCDMOD1 = 1 ;		//LCD Mode All OFF
			FLBTEN = 0 ;		//关低电压检测
			FTC0ENB = 0;		//关TC0 timer
			FTC1ENB = 0;		//关TC1 timer
			FTC2ENB = 0;		//关TC2 timer
			INTEN0 = 0;			//所有中断除能
			INTEN1 = 0;			//所有中断除能
			FGIE = 0 ;			//关总中断
			NOP(2);

			//无按键按下
			l_buf = 1;
			while(l_buf)
			{
				FCPUM0 = 1;       // CPU 停止，等 WDT 唤醒
   				 NOP(2);            // WDT 唤醒后从这里继续（FCPUM0 已自动清零）
				while( Port_Mem && Port_Test && Port_Set )
				{
					WDTR = 0X5A;	//喂狗
				}
				 Delay50us(4);	//green mode fcpu=32768/4=8k，是normal模式的250分之一，故实际延迟去抖为50ms
				 
				 if( !Port_Mem || !Port_Test || !Port_Set )
				 {
					l_buf = 0;
				 }
			}


			FSTPHX = 0;		//开IHRC
			NOP(2);
			FCLKMD = 0;		//关Slow模式
			NOP(2);

			GPIO_Init();	//IO口设置
			Lcd_Init();		//Lcd设置
			TC1Init();		//TC1设置（10ms）
			FGIE = 1;

			HalKey_KeyClr();
			HalKey_Set_KeyMode(Func_Short, &sMemKey);
			HalKey_Set_KeyMode(Func_Long, &sTestKey);
			HalKey_Set_KeyMode(Func_Short, &sModeKey);
			HalKey_Set_KeyMode(Func_Long, &sSetKey);
			l_buf = 0;
			eSleepTask_Sta = Sleep_wakeup;
			break;

		case Sleep_wakeup:
			l_buf ++;
			if( l_buf > 8 )
			{
				if( uKeyPress.bits.SKeyPress && uSetFlag.bits.Unit_Change == Unit_Change_En)
				{
					if( uKeyHold.bits.SKeyHold  && !uStaFlag.bits.LowBat)
					{
						if( uSetFlag.bits.Voiceflag )
						{
							#if Have_Voice_Func
								AM5BA_SPI_Init();
								AM5BA_Power_Enable();
								PlayStatueParam(2 , 0 , 0);
								PlayStatueParam(1 , Play_Di,0);
								App_PlayVoice3();
								Delay10ms(1);
							#else
								BZ_Beep230();
							#endif	
						}
						if( uSetFlag.bits.Motorflag == 1 )
						{
							MT_Vib230();			
						}
						Auto_TurnOff_Time_Sel();
						eSleepTask_Sta = Sleep_false;
						eMain_Task = Task_Unitmode;
					}
					else if( uKeyHold.bits.SKeyHold  && uStaFlag.bits.LowBat )
					{
						Clr_Disp();
						Disp_LowBat();
						Auto_TurnOff_Time_Sel();
						eMain_Task = Task_InitMode;
						eInitTask_Sta = Init_Err;
						eSleepTask_Sta = Sleep_false;
					}
				}
				else if( uKeyPress.bits.MemKeyPress && !uStaFlag.bits.LowBat )
				{
					eReadyTask_Sta = Ready_ReadyOk;
					eMain_Task = Task_Memorymode;
					Auto_TurnOff_Time_Sel();
					#if Secondary_voltage
						LVD_Display();
					#else
						Disp_FullBat();
					#endif
				}
				else if( uKeyPress.bits.TKeyPress )
				{
					eSleepTask_Sta = Sleep_End;
				}
			}
			break;

		case Sleep_End:
			eSleepTask_Sta = Sleep_false;
			eMain_Task = Task_InitMode;
			HalKey_Set_KeyMode(Func_Short, &sMemKey);
			HalKey_Set_KeyMode(Func_Long_Long, &sTestKey);
			HalKey_Set_KeyMode(Func_Short, &sModeKey);
			HalKey_Set_KeyMode(Func_Long, &sSetKey);
			#if Have_Voice_Func
				AM5BA_SPI_Init();
				AM5BA_Power_Enable();
				PlayStatueParam(2 , 0 , 0);
				PlayStatueParam(1 , Play_VolumeMax,0);
			#endif
			break;

		default:
			break;
	}
}
