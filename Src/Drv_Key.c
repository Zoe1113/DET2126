/**************************************************************************
文件名称：	Drv_Key.c
说    明：	按键驱动函数集合（驱动层、应用层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
备	  注:	按键以release作为触发
修订记录：
**************************************************************************/
#include "Include.h"

//按键状态eKeySta
enum eKeyStatus
{
	KeySta_Init = 0,	//按键初始化状态
	KeySta_Dither = 1,	//按键去抖状态
	KeySta_Comfirm = 2,	//按键确认状态
	KeySta_Release = 3	//按键释放状态
} eKeySta;

//按键键值eKeyVal
enum eKeyValue
{
	MemKey = 0x01,		 //记忆键
	TestKey = 0x02,		 //测试/测量键
	SetKey = 0x10,		 //设置键
	ModeKey = 0x20,		 //模式/探头键
	KeyMask = 0x33		 //按键掩码（p00 p01 p04 p05）
}eKeyVal;

uKey1 uKeyPress;
uKey2 uKeyHold;
uKey3 uKeyRelease;
uKey4 uKeyContinue;

strKey sMemKey, sTestKey, sSetKey, sModeKey;

uint8 SKeyHoldFlag = 0;//长按为1，短按为0


/**************************************************************************
    函数名称：	void TKeyProcess(void)
    函数功能：	Reaymode下测试键处理，开始测量
    输入参数：	
    输出参数：	无
    返回值  ：	无
    占用空间：	TBD
****************************************************************************/
void TKeyProcess(void)
{
	//在ready模式，准备OK的情况下按下测试键开始测量
	if((uKeyRelease.bits.TKeyRelease && eReadyTask_Sta == Ready_ReadyOk && !uErrFlag.bits.Er2&&eMain_Task == Task_ReadyMode)||(eTestmode_num==Insptectmode&&uKeyRelease.bits.TKeyRelease&&eMain_Task == Task_Memorymode))
	{
		
		lcd_mem_clr();
	    #if Have_Voice_Func  //关闭语音
            PlayStatueParam(2 , 0 , 0);
            PlayStatueParam(1 , Play_Stop,0);
        #endif
		
		Er3_Display_Sound(RESET);

		Time_CountDown_5s_timeout(RESET);
		Auto_TurnOff_Time_Sel();	//测量开始时重置自动关机计时
        eMain_Task = Task_Testingmode;
		//g_50ms_Count = DispTime_Init;	//循环显示时间、温度、ntc实时时间等初始化值
		Disp_Unit();	//显示单位
	}
	//长按3s关机，进入睡眠模式
	if(uKeyPress.bits.TKeyPress)
	{
		if(uKeyHold.bits.TKeyHold)
		{
			if(uSetFlag.bits.Voiceflag ==1)
			{
				#if Have_Voice_Func
					/*关闭语音播报*/
					PlayStatueParam(2 , 0 , 0);
					PlayStatueParam(1 , Play_Stop,0);
					AM5BA_Send_Cmd(LL_Stop, SS_Stop, Nodata);
				#else
					BZ_Beep230();
				#endif
			}
			if( uSetFlag.bits.Motorflag == 1 )
			{
				MT_Vib230();			
			}
			eMain_Task = Task_Sleepmode;
			eSleepTask_Sta = Sleep_false;
			HalKey_Set_KeyMode(Func_Long, &sTestKey);	//测量键设为长按
			sTestKey.g_Key_Hold_cnt = 0;	//测试键计时清零，确保下次长按3s
			uKeyHold.bits.TKeyHold = 0;
		}
	}
	//温度不稳定时或超时状态下，测试键释放后进入Er1的5秒等待；等待未完成时再次释放测试键则重新开始等待
	if(eTestmode_num != Foreheadmode && eTestmode_num != Objectmode)
	{
		if(uKeyRelease.bits.TKeyRelease && (eReadyTask_Sta == Ready_DisEr1 || eReadyTask_Sta == Ready_Timeout))
		{
			#if Have_Voice_Func  //关闭语音
				PlayStatueParam(2 , 0 , 0);
				PlayStatueParam(1 , Play_Stop,0);
			#endif
			//Time_CountDown_5s_timeout(RESET);
			
			Adc_Channel_Init(TPTONTC);			//切换到ntc通道
			eReadyTask_Sta = Ready_DisEr1;
			uKeyRelease.bits.TKeyRelease = 0;
		}
	}
	if(!uKeyPress.bits.TKeyPress)
	{
		uKeyRelease.bits.TKeyRelease = 0;
	}
	
}

void MemKeyProcess(void)
{
	static bit S_MemKeyAgeSwitched = 0;	//年龄切换已触发标志，防止长按重复触发

	//按下记忆键切换年龄分段（按下触发，长按只触发一次）
	if( uKeyPress.bits.MemKeyPress && !S_MemKeyAgeSwitched && eMain_Task == Task_ReadyMode && (eReadyTask_Sta == Ready_ReadyOk || eReadyTask_Sta == Ready_Timeout) && eTestmode_num!=Objectmode && eTestmode_num!=Insptectmode )
	{
		S_MemKeyAgeSwitched = 1;
		uErrFlag.bits.Er2 = 0;
		uErrFlag.g_ErrFlag = 0;
		
		//循环切换年龄分段
		switch(g_AgeGroup)
		{
			case AgeGroup_0_3:
				g_AgeGroup = AgeGroup_3_36;
				break;
			case AgeGroup_3_36:
				g_AgeGroup = AgeGroup_36_Plus;
				break;
			case AgeGroup_36_Plus:
			default:
				g_AgeGroup = AgeGroup_0_3;
				break;
		}
		Disp_Ready();	//待机画面显示_ _._
		Disp_ModeSign();//模式标志
		LED_CloseAll();
		LED_Green_En();		//绿色背光
		
		//蜂鸣提示
		if(uSetFlag.bits.Voiceflag == 1)
		{
			#if Have_Voice_Func
				PlayStatueParam(2, 0, 0);
				PlayStatueParam(1, Play_Stop, 0);
				g_DiDo = g_AgeGroup + 1;
				PlayStatueParam(1, Play_MemNum, 0);
			#else
				BZ_Beep230();
			#endif
		}
		if( uSetFlag.bits.Motorflag == 1 )
		{
			MT_Vib230();			
		}
		g_15s_Count = CountDown_15s;
		Auto_TurnOff_Time_Sel();
		if( eReadyTask_Sta != Ready_Timeout )
		{
			eReadyTask_Sta = Ready_ReadyOk;
		}
	}

	//按键释放后复位标志，允许下次按下再次触发
	if( !uKeyPress.bits.MemKeyPress )
	{
		S_MemKeyAgeSwitched = 0;
	}

	if( uKeyPress.bits.MemKeyPress && eTestmode_num==Insptectmode )
	{
		eReadyTask_Sta = Ready_ReadyOk;
		eMain_Task = Task_Memorymode;
		Auto_TurnOff_Time_Sel();	//初始化自动关机计时器
		
	}
}


//set键处理
void SkeyProcess(void)
{
	#if Func_Obj
	static int8 L_mode_buf;		//模式切换标志(根据客户产品需求)

	//检测到长按set按键并且不在显示ER2且不在记忆模式且探头盖合上 则进行模式切换，额温模式切换到物温模式或物温模式
	if((eTestmode_num == Objectmode||eTestmode_num == Foreheadmode)&& (uKeyPress.bits.SKeyPress && eReadyTask_Sta != Ready_DisEr2 && eMain_Task != Task_Memorymode && uStaFlag.bits.ProbeCover == 1))
	{
		if( uKeyHold.bits.SKeyHold && !uKeyRelease.bits.SKeyRelease )
		{
			SKeyHoldFlag = 1;
			uKeyHold.bits.SKeyHold = 0;
			uKeyRelease.bits.SKeyRelease = 1;
			Auto_TurnOff_Time_Sel();	//自动关机时间清零
			if( eTestmode_num == Foreheadmode )
			{
				eTestmode_num = Objectmode;
				Adc_Channel_Init(TPTONTC);
			}
			else if(eTestmode_num == Objectmode)
			{
				eTestmode_num = Foreheadmode;
				Adc_Channel_Init(TPTONTC);
			}
			if(uSetFlag.bits.Voiceflag ==1)
			{
				#if Have_Voice_Func
					uSetFlag.bits.Ready_First = 0;
					PlayStatueParam(2 , 0 , 0);
					PlayStatueParam(1 , Play_Stop,0);
					PlayStatueParam(1 , Play_Clean,0);
				#else
					uSetFlag.bits.Ready_First = 1;
				#endif
			}
			if( uSetFlag.bits.Motorflag == 1 )
			{
				uSetFlag.bits.Ready_First = 1;			
			}
			eReadyTask_Sta = Ready_Refresh;
			eMain_Task = Task_ReadyMode;
		}
	}
	#endif
	//set键短按切换语音开关和马达开关
	else
	{	if(uErrFlag.bits.Er2)
		{
			return;
		}
		if(uKeyHold.bits.SKeyHold && !uKeyRelease.bits.SKeyRelease)
		{
			SKeyHoldFlag = 1;
		}
		if(uKeyRelease.bits.SKeyRelease&&SKeyHoldFlag)
		{
			SKeyHoldFlag = 0;
			uKeyRelease.bits.SKeyRelease=0;
		}
		else if(uKeyRelease.bits.SKeyRelease && !SKeyHoldFlag)
		{
			uSetFlag.bits.Voiceflag = ~uSetFlag.bits.Voiceflag;
			if(uSetFlag.bits.Voiceflag==1)
			{
				uSetFlag.bits.Motorflag=0;
			}
			else
			{
				uSetFlag.bits.Motorflag=1;
			}
			Disp_VoiceSign();
			// 语音打开时播放提示音 
			if( uSetFlag.bits.Voiceflag == 1 )
			{
				#if Have_Voice_Func
					PlayStatueParam(2 , 0 , 0);
					PlayStatueParam(1 , Play_Di,0);
				#else
					BZ_Beep230();
				#endif					
			}
			if( uSetFlag.bits.Motorflag == 1 )
			{
				MT_Vib230();			
			}
			// 语音关闭时停止正在播放的内容
			else
			{
				#if Have_Voice_Func

					PlayStatueParam(2 , 0 , 0);
					AM5BA_Send_Cmd(LL_Stop, SS_Stop, Nodata);

				#endif
			}
			Auto_TurnOff_Time_Sel();	//自动关机时间清零
		}
	}
	if(!uKeyPress.bits.SKeyPress)
	{
		uKeyRelease.bits.SKeyRelease = 0;
	}
}



//探头按键
void App_PCKeyProcess(void)
{
	if( uKeyPress.bits.MKeyPress )
	{
		uKeyRelease.bits.MKeyRelease = 0;
	}
	if ( uKeyPress.bits.MKeyPress&&eTestmode_num == Earmode&&!uErrFlag.bits.Er2)
    {
		uKeyPress.bits.MKeyPress=0;
       	eTestmode_num=Foreheadmode;
	   	Adc_Channel_Init(TPTONTC);
	  	uStaFlag.bits.ProbeCover = 1;
	  	eReadyTask_Sta = Ready_Refresh;
		eTestTask_Sta=Test_Init;
		eMain_Task = Task_ReadyMode;
		Auto_TurnOff_Time_Sel();//自动关机
    }

    else if ( uKeyRelease.bits.MKeyRelease&&(eTestmode_num == Foreheadmode||eTestmode_num == Objectmode) &&!uErrFlag.bits.Er2)
    {
		uKeyRelease.bits.MKeyRelease=0;
		Disp_FourSecLoop_Init();
		uSetFlag.bits.Ready_First = 0;
		eTestmode_num=Earmode;
		Adc_Channel_Init(NTCTOTP);	//切回耳温后立即切到TP通道，避免Ready流程前短暂采样窗口误判Er2
	  	uStaFlag.bits.ProbeCover = 0;
		eReadyTask_Sta = Ready_Refresh;
		eTestTask_Sta=Test_Init;
		eMain_Task = Task_ReadyMode;
		Auto_TurnOff_Time_Sel();//自动关机
    }
}



/**************************************************************************
函数名称：	uint8 HalKey_ReadKeyVal(void)
函数功能：	读取按键键值
输入参数：	P0
输出参数：	无
返回值  ：	L_keydata按键键值变量
占用空间：	TBD
备    注:	无
**************************************************************************/
uint8 HalKey_ReadKeyVal(void)
{
	uint8 L_keydata;
	L_keydata = P0 & 0x13;	//只读取P00,P01,P04，P05不再使用
	L_keydata ^= 0x13;	//低电平有效按键取反
	
	//读取P22作为模式键
	if(!FP22)	//P22低电平表示按键按下
	{
		L_keydata |= ModeKey;	//设置模式键标志
	}
	return L_keydata;
}

/**************************************************************************
函数名称：	void HalKey_KeyScan(void)
函数功能：	扫描按键
输入参数：	无
输出参数：	uOTKeyFlag，uSMKeyFlag
返回值  ：	无
占用空间：	TBD
备    注:	无
**************************************************************************/

void HalKey_KeyScan(void)
{
	uint8 L_Keydata;
	L_Keydata = HalKey_ReadKeyVal();
	HalKey_Scan(L_Keydata & MemKey, &sMemKey);
	HalKey_Scan(L_Keydata & SetKey,  &sSetKey);
	HalKey_Scan(L_Keydata & TestKey, &sTestKey);
	HalKey_Scan(L_Keydata & ModeKey, &sModeKey);
}

/**************************************************************************
函数名称：	void HalKey_KeyScan(void)
函数功能：	扫描按键
输入参数：	无
输出参数：	uOTKeyFlag，uSMKeyFlag，uKeyRelease，g_KeyContinue_Flag，g_KeyPress_Flag，g_KeyHold_Flag
返回值  ：	无
占用空间：	TBD
备    注:	uKeyRelease为按键释放标志
**************************************************************************/
void HalKey_Scan(uint8 L_Keydata, strKey *sKey)
{
	switch (sKey->g_Key_Status)
	{
		//按键等待状态
		case KeySta_Init:
			if (L_Keydata) //检测到按键按下
			{
				sKey->g_Key_Hold_cnt = 0;
				sKey->g_Key_Val = L_Keydata;
				// uKeyRelease.g_KeyRelease_Flag &= ~L_Keydata;		//抬起按键清除标志
				sKey->g_Key_Status = KeySta_Dither;
			}
			break;
		//按键去抖状态
		case KeySta_Dither:
			if ( sKey->g_Key_Val == L_Keydata )
			{
				sKey->g_Key_Hold_cnt ++;
				if ( sKey->g_Key_Hold_cnt > CNT_Dither)
				{
					sKey->g_Key_Status = KeySta_Comfirm;
					uKeyPress.g_KeyPress_Flag |= L_Keydata;	//设置按键按下标志位
				}
			}
			else
			{
				sKey->g_Key_Status = KeySta_Init;
			}
			break;
		//按键确认状态
		case KeySta_Comfirm:
			if (L_Keydata == sKey->g_Key_Val)
			{
				if (sKey->g_Key_Hold_cnt < 0xFE) //按键计数从0开始
				{
					sKey->g_Key_Hold_cnt ++;
				}
				if (sKey->g_Key_Hold_cnt > sKey->g_Key_preset_cnt)
				{
					uKeyHold.g_KeyHold_Flag |= L_Keydata;	//设置按键长按标志位
					//开启连续操作
					if ( sKey->g_KeyFun & En_Cp )
					{
						uKeyContinue.g_KeyContinue_Flag |= L_Keydata;	//设置按键连按标志位
						sKey->g_Key_Hold_cnt = sKey->g_Key_preset_cnt - CNT_CPInterval;
					}
				}
			}
			else
			{
				sKey->g_Key_Status = KeySta_Release;
			}
			break;
		//按键释放状态
		case KeySta_Release:
			if (!L_Keydata)
			{
				uKeyHold.g_KeyHold_Flag &= ~sKey->g_Key_Val;
				uKeyPress.g_KeyPress_Flag &= ~sKey->g_Key_Val;
				uKeyContinue.g_KeyContinue_Flag &= ~sKey->g_Key_Val;
				uKeyRelease.g_KeyRelease_Flag |= sKey->g_Key_Val;
				sKey->g_Key_Status = KeySta_Init;
			}
			break;
		default:
			break;
	}
}

/**************************************************************************
函数名称：	void HalKey_Set_KeyMode(uint8 function, struct strKey *sKey )
函数功能：	设置按键工作模式
输入参数：	mode短按/长按/短长按/连按/未设定默认为长按
输出参数：	sKey->g_KeyFun按键功能位，sKey->g_Key_preset_cnt按键预设计数值变量
返回值  ：	无
占用空间：	TBD
备    注:	无
**************************************************************************/
void HalKey_Set_KeyMode(uint8 function, strKey *sKey )
{
	switch (function)
	{
		case Func_Short:
			sKey->g_KeyFun &= ~En_Cp; 	//禁用连按
			sKey->g_Key_preset_cnt = CNT_Invalid;
			break;
		case Func_Short_Long:
			sKey->g_KeyFun &= ~En_Cp;	//禁用连按
			sKey->g_Key_preset_cnt = CNT_ShortLong;
			break;
		case Func_Long:
			sKey->g_KeyFun &= ~En_Cp;	//禁用连按
			sKey->g_Key_preset_cnt = CNT_LongPress;
			break;
		case Func_Short_Continue:
			sKey->g_KeyFun |= En_Cp; 	//启用连按
			sKey->g_Key_preset_cnt = CNT_EnterCP;
			break;
		case Func_Long_Long:
			sKey->g_KeyFun &= ~En_Cp; 	//禁用连按
			sKey->g_Key_preset_cnt = CNT_LongLongPress;
			break;
		default:
			sKey->g_KeyFun &= ~En_Cp; 	//禁用连按
			sKey->g_Key_preset_cnt = CNT_LongPress;
			break;
		}
}

/**************************************************************************
函数名称：	void HalKey_KeyClr(void)
函数功能：	清除所有按键信息（被外部调用，作用点外部）
输入参数：	g_KeyHold_Flag，g_KeyPress_Flag，g_KeyRelease_Flag，g_KeyContinue_Flag，g_Key_Val键值变量，g_Key_Status按键状态变量
输出参数：	g_KeyHold_Flag，g_KeyPress_Flag，g_KeyRelease_Flag，g_KeyContinue_Flag，g_Key_Val键值变量，g_Key_Status按键状态变量
返回值  ：	无
占用空间：	TBD
备    注:	无
**************************************************************************/
void HalKey_KeyClr(void)
{
	uKeyHold.g_KeyHold_Flag = 0;
	uKeyPress.g_KeyPress_Flag = 0;
	uKeyRelease.g_KeyRelease_Flag = 0;
	uKeyContinue.g_KeyContinue_Flag = 0;
	sMemKey.g_Key_Val = 0;
	sTestKey.g_Key_Val = 0;
	sSetKey.g_Key_Val = 0;
	sModeKey.g_Key_Val = 0;
	sMemKey.g_Key_Status = 0;	
	sTestKey.g_Key_Status = 0;
	sSetKey.g_Key_Status = 0;
	sModeKey.g_Key_Status = 0;
}