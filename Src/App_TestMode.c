/**************************************************************************
文件名称：	App_TestMode.c
说    明：	测试模式（应用层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
备    注:
修订记录：
**************************************************************************/
#include "Include.h"

eTestTaskSta eTestTask_Sta;
eTestmode eTestmode_num;

bit F_Disp_Dash;	//显示虚线标志位
static uint16 g_Forehead_measure_time;	//额温测量时间计数器
static int16 g_TpStep_max;	//温度最大值缓存
static int16 g_TpStep_Disp;  //目标温度0.1度单位


void App_TestingMode(void)
{
	switch(eTestTask_Sta)
	{
		//测试初始化，测量开始的准备工作
		case Test_Init:
			#if Func_debug
				Uart_Transmit(0xCC, 0xCCCC);
			#endif
			//关闭LED
			LED_CloseAll();
			Clr_ModeSign();
			lcd_badface_clr();
			lcd_smileface_clr();
			uStaFlag.bits.Fever = 0;	//发烧标志位
			F_Mem_FirstEnter = 0; //清除记忆
			
			#if Have_Voice_Func
				PlayStatueParam(2 , 0 , 0); 
                PlayStatueParam(1 , Play_Stop,0);
			#endif
			
			
			//保存按下时ntc数据
			Ntc_Caculate();
			NtcTable_Check();
			if( eTestmode_num == Insptectmode )
			{
				NtcTableWider_Check();
			}
			if( uErrFlag.bits.Er2)
			{
				Clr_Disp888();
				Disp_ErN(2);
				eTestTask_Sta = Test_Disp;
			}
			else
			{
				//保存测试参数
				Backup_Testing_Param();
				//非耳温模式，需要切换tp
				if( eTestmode_num != Earmode )
				{
					g_TpStep_max = 0x8000;	//赋最小值
					Adc_Channel_Init(NTCTOTP);//ADC初始化通道切换
					F_First_Enter_ForeheadTp = 0;
					F_First_Enter_EarTp = 0;
				}
				else
				{
					g_AdcCount = 0;
				}

				//选择额温测量方法
				#if !Forehead_Measure_Method
					if( eTestmode_num == Foreheadmode )
					{
						g_Forehead_measure_time = 3;
						F_Disp_Dash = Enable;
						g_300ms_Count = 0;
						Disp_ForeheadLoading_Init();
					}
				#endif
				eTestTask_Sta = Test_Get_Tp;
			}
			#if Func_debug
				Uart_Transmit(0x90, g_NtcCount);
			#endif
			break;
			
		//检测thermopile电压
		case Test_Get_Tp:
			//额温模式下并行处理加载动画
			if( eTestmode_num == Foreheadmode )
			{
				Disp_ForeheadLoading_Process();
			}
			if( Get_Adc_SingleRead() )
			{
				g_AdcCount ++;
				if( eTestmode_num == Earmode )
				{
					Get_Ear_Tp_Max( g_AdcData );		//取峰值
					if( g_AdcCount == Ear_Num )
					{
						if( Probe_Move(Ear_array,Ear_Num) )
						{
							uErrFlag.bits.Er3 = 0;
							g_AdcCount = Adc_Filter(Ear_array, Ear_Num);
							g_TpCount_avg = Get_Ear_Tp_Avg(g_AdcCount);		//计算耳温平均最大值
							//g_TpCount = TpCount_Determine();		//目标温度点确定算法
							g_AdcCount = 0;		//清0保证下次测量重新循环
							g_TpCount = g_TpCount_avg;
							Adc_Channel_Init(TPTONTC);//ADC初始化通道切换
							eTestTask_Sta = Test_Get_Ntc;
						}
						else
						{
							uErrFlag.g_ErrFlag = 0;		//清除错误标志
							uErrFlag.bits.Er3 = 1;
							eTestTask_Sta = Test_Disp;
						}
					}
				}
				else if( eTestmode_num == Foreheadmode) 
				{
					Get_Forehead_Tp( g_AdcData );
					if( g_AdcCount == Forehead_Num )
					{
						g_AdcCount = Remove_Tp_Max_Min( Forehead_array, g_AdcCount );//去掉最大最小值
						g_AdcCount = Adc_Filter(Forehead_array, g_AdcCount);
						g_TpCount = Get_Forehead_Tp_Max_Avg(g_AdcCount);	//计算最大平均值
						g_AdcCount = 0;		//清0保证下次测量重新循环
						
						eTestTask_Sta = Test_Calculate;		//额温检测完成计算ntc
					}
				}
				else
				{
					//其他模式只做平均值，不做峰值，Get_Ear_Tp_Max获取原始值处理
					Get_Ear_Tp_Max( g_AdcData );
					if( g_AdcCount == Ear_Num )
					{
						if( Probe_Move(Ear_array,Ear_Num) )
						{
							uErrFlag.bits.Er3 = 0;
							g_AdcCount = Adc_Filter(Ear_array, Ear_Num);
							g_TpCount_avg = Get_Ear_Tp_Avg(g_AdcCount);
							g_AdcCount = 0;		//清0保证下次测量重新循环
							g_TpCount = g_TpCount_avg;
							Adc_Channel_Init(TPTONTC);
							eTestTask_Sta = Test_Get_Ntc;
						}
						else
						{
							uErrFlag.g_ErrFlag = 0;		//清除错误标志
							uErrFlag.bits.Er3 = 1;
							eTestTask_Sta = Test_Disp;
						}
					}
				}
			}
			break;

		//检测环境和探头ntc温度
		case Test_Get_Ntc:
			if( Get_Ntc_Count() )
			{
				Ntc_Caculate();
				NtcTable_Check();
				if( eTestmode_num == Insptectmode )
				{
					NtcTableWider_Check();
				}
				//判断是否Er2错误
				if( uErrFlag.bits.Er2 )
				{
					eTestTask_Sta = Test_Disp;
				}
				else
				{
					//ntc判定
					g_NtcCount = NtcCount_Determine();
					eTestTask_Sta = Test_Calculate;
				}
			}
			break;

		//计算最终温度
		case Test_Calculate:

		#if Func_debug
			Uart_Transmit(0x91, g_NtcCount);
		#endif

		#if Func_debug
			Uart_Transmit(0x92, g_TpCount);
		#endif
			
			NtcTable_Find();

		#if Func_debug
			Uart_Transmit(0x93, g_NtcStep);
		#endif

			Tp_Caculate();
		#if Func_debug
			Uart_Transmit(0x94, g_TpCount);
		#endif

			Emissivity_correction();

		#if Func_debug
			Uart_Transmit(0x95, g_TpCount);
		#endif
			TpTable_Find();
			// g_NtcStep=2500;
			// g_TpStep=3700;
		#if Func_debug
			Uart_Transmit(0x96, g_TpStep);
		#endif
		
			BlackBodyOffset();

		#if Func_debug
			Uart_Transmit(0x97, g_TpStep);
		#endif
			eTestTask_Sta = Test_Disp;	//默认显示状态
			
			switch( eTestmode_num )
			{
				case Earmode:
					#if Func_Probecover
					if( uStaFlag.bits.ProbeCover )
						Probecover_compensate();
					#endif
					Ear_Compensate();
					Temp_Relate();
					LowTemp_Compensate();
					Body_MeasureRange_Check();
					g_TpStep_Disp = CToF(g_TpStep);
					g_TpStep_Disp = Temp_Resolution_Adjust(g_TpStep_Disp);
  					//Fever_alarm(g_TpStep_Disp);
					break;
				case Foreheadmode:
					
					Forehead_Compensate();
					Temp_Relate();
				#if Func_debug
					Uart_Transmit(0x98, g_TpStep);
				#endif
					//选择额温测量方法
				#if Forehead_Measure_Method
					/*******额温连续测量模式代码********/
					if( g_TpStep > g_TpStep_max )
					{
						g_TpStep_max = g_TpStep;
					}
					Body_MeasureRange_Check();
					if( uKeyPress.bits.TKeyPress && !uErrFlag.bits.Hi )
					{
						g_TpStep = g_TpStep_max;
						Body_MeasureRange_Check();
						if( uErrFlag.bits.Lo )
						{
							Disp_Lo();
						}
						else
						{
							g_TpStep_Disp = CToF(g_TpStep_max);
							g_TpStep_Disp = Temp_Resolution_Adjust(g_TpStep_Disp);
							Disp_Temp(1,0,uSetFlag.bits.Unit,g_TpStep_Disp);	//显示温度
						}
						eTestTask_Sta = Test_Get_Tp;
					}
					else
					{
						F_First_Enter_ForeheadTp = 0;	//清0保证下次测量重新检测
						g_TpStep = g_TpStep_max;
						Body_MeasureRange_Check();
						g_TpStep_Disp = CToF(g_TpStep);
						g_TpStep_Disp = Temp_Resolution_Adjust(g_TpStep_Disp);
						//Fever_alarm(g_TpStep_Disp);
						eTestTask_Sta = Test_Disp;
					}
					/************************************/
				#else
					/********** 额温固定N次代码 **********/
					if( g_TpStep > g_TpStep_max )
					{
						g_TpStep_max = g_TpStep;
					}
					g_Forehead_measure_time --;
					if( g_Forehead_measure_time )
					{
						#if Func_debug
							Uart_Transmit(0x99, g_TpStep_max);
						#endif
							eTestTask_Sta = Test_Get_Tp;
					}
					else
					{
						F_Disp_Dash = Disable;	//停止刷新
						F_First_Enter_ForeheadTp = 0;	//清0保证下次测量重新检测
						g_TpStep = g_TpStep_max;
						Body_MeasureRange_Check();
						g_TpStep_Disp = CToF(g_TpStep);
						g_TpStep_Disp = Temp_Resolution_Adjust(g_TpStep_Disp);
						//Fever_alarm(g_TpStep_Disp);
						eTestTask_Sta = Test_Disp;
					}
					/***********************************/
				#endif
					break;
				case Objectmode:	
					Temp_Relate();
					Obj_MeasureRange_Check();
					if( uKeyPress.bits.TKeyPress && !uErrFlag.bits.Hi )
					{
						Obj_MeasureRange_Check();
						if( uErrFlag.bits.Lo )
						{
							Disp_Lo();
						}
						else
						{
							g_TpStep_Disp = CToF(g_TpStep);
							g_TpStep_Disp = Temp_Resolution_Adjust(g_TpStep_Disp);
							Disp_Temp(1,0,uSetFlag.bits.Unit,g_TpStep_Disp);	//显示温度
						}
						eTestTask_Sta = Test_Get_Tp;
						Adc_Channel_Init(NTCTOTP);
					}
					else
					{
						F_First_Enter_ForeheadTp = 0;	//清0保证下次测量重新检测
						//g_TpStep = g_TpStep_max;
						Obj_MeasureRange_Check();
						g_TpStep_Disp = CToF(g_TpStep);
						g_TpStep_Disp = Temp_Resolution_Adjust(g_TpStep_Disp);
						LED_Green_En();
						eTestTask_Sta = Test_Disp;
					}
					break;
				case Blackbodymode:
					#if Func_Probecover
					if( uStaFlag.bits.ProbeCover )
						Probecover_compensate();
					#endif
					Temp_Relate();
					Body_MeasureRange_Check();
					g_TpStep_Disp = CToF(g_TpStep);
					g_TpStep_Disp = Temp_Resolution_Adjust(g_TpStep_Disp);
					//Fever_alarm(g_TpStep_Disp);
					break;
				case Insptectmode:
					Obj_MeasureRange_Check();
					g_TpStep_Disp = g_TpStep;
					break;
				default:
					break;
			}
			break;
		//显示结果
		case Test_Disp:
			
			if(g_ForeheadLoading_Sta==0&&eTestmode_num == Foreheadmode)
			{
				//停止额温加载动画
				Disp_ForeheadLoading_Stop();
			}
			else if(g_ForeheadLoading_Sta!=0&&eTestmode_num == Foreheadmode)
			{
				Disp_ForeheadLoading_Process();
				   break;
			}
			Clr_Disp888();
			Disp_ModeSign();
			//错误信息优先级为：错误标志位Er2>Lo/Hi>Er3/Er4等
			if( uErrFlag.g_ErrFlag )
			{
				Disp_ErrMsg();	//仅显示哭笑脸 + 错误符号
				switch( uErrFlag.g_ErrFlag )
				{
					case 4:		// Er3 探头错误
						LED_CloseAll();
						Er3_Display_Sound(RUN);	// Er3 报警一次
						break;
					case 0x40:	// Lo 低温
						if( eTestmode_num != Insptectmode )
						{
							LED_Green_En();
						}
						else
						{
							first_InsReady = 0;	// 生产检验模式下复位就绪标志
						}
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
						break;
					case 0x80:	// Hi 高温
						if( eTestmode_num != Insptectmode && eTestmode_num != Objectmode )
						{
							LED_Red_En();
						}
						else if(eTestmode_num == Objectmode)
						{
							LED_CloseAll();
						    LED_Green_En();
						}
						else if( eTestmode_num == Insptectmode )
						{
							first_InsReady = 0;	// 生产检验模式下复位就绪标志
						}
						if( uSetFlag.bits.Voiceflag == 1 )
						{
							BZ_Beep230();
							BZ_Beep160();
							BZ_Beep160();
							BZ_Beep160();
						}
						if( uSetFlag.bits.Motorflag == 1 )
						{
							MT_Vib230();
							MT_Vib160();
							MT_Vib160();
							MT_Vib160();
						}
						break;
					default:
						break;
				}
			}
			else
			{
				if( eTestmode_num == Insptectmode )
					Disp_Temp(0,1,uSetFlag.bits.Unit,g_TpStep_Disp);	//显示温度
				else
					Disp_Temp(1,0,uSetFlag.bits.Unit,g_TpStep_Disp);	//显示温度
				Mem_Store(g_TpStep);	//存储到记忆(0.01摄氏度)
			}
	
			if(eTestmode_num != Objectmode&&eTestmode_num !=Insptectmode&&!uErrFlag.g_ErrFlag)
			{
				Fever_alarm(g_TpStep_Disp);
			}
			else if((eTestmode_num == Objectmode||eTestmode_num ==Insptectmode)&&!uErrFlag.g_ErrFlag)
			{
				if(uSetFlag.bits.Voiceflag == 1)
				{
					BZ_Beep230(); //Bi-
				}
				if(uSetFlag.bits.Motorflag == 1)
				{
					MT_Vib230();			
				}
			}
			eTestTask_Sta = Test_End;
			break;
		//测试结束，返回Task_Ready等待测试模式
		case Test_End:
			Adc_Channel_Init(TPTONTC);			//切换到ntc通道
			eTestTask_Sta = Test_Init;		//当前测试状态初始化
			eMain_Task = Task_ReadyMode;	//进入下一任务
			eReadyTask_Sta = Ready_Timeout;	//下一任务状态初始化
			g_15s_Count = CountDown_15s;	//记忆显示15秒倒计时
			uKeyRelease.bits.TKeyRelease = 0;	//清除本次测量键释放标志，等待下一次实际释放
			break;

		default:
			break;
	}
	App_PCKeyProcess();
}