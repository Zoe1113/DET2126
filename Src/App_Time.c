/**************************************************************************
文件名称：	App_DispTime.c
说    明：	显示时间函数（应用层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
备    注:
修订记录：
**************************************************************************/
#include "Include.h"

//变量定义
bit F_10ms;		//10ms中断标志位（TC1产生）
bit F_10ms_task;//10ms中断标志位（TC1产生）
bit F_20ms;		//20ms中断标志位（10ms中断生成）
bit F_50ms;		//50ms中断标志位（10ms中断生成）
bit F_500ms;	//500ms中断标志位（T0产生）
bit F_5s_TimeOut;	//5s到标志位，1：5s到，0：5s未到
bit F_LED_Enable;	//三色背光开启标志位，0：除能，1：使能
bit F_Colon_Blink;	//冒号闪烁使能，0：除能，1：使能

eDispTime g_50ms_Count;

uint8 g_10ms_Count;		//10ms计数器
uint8 g_20ms_Count;		//20ms计数器
uint8 g_300ms_Count;	//300ms计数器

uint16 g_15s_Count;		//15s计数器
uint16 g_5s_Count;		//5s计数器
uint16 g_AutoTurnOff_Count;	//自动关机计数器
uint8 g_PreserveSpecialMode;	//自动关机时保留生产/黑体模式标志


/**************************************************************************
函数名称：	void Time_Creat_20ms_50ms(void)
函数功能：	利用10ms基础定时器裂变生成20ms、50ms等定时时间
输入参数：	无
输出参数：	无
返回值  ：	无
占用空间：	TBD
备    注：	10ms作为时基
**************************************************************************/
void Time_Creat_20ms_50ms(void)
{
	g_10ms_Count++;

	if(g_10ms_Count%2 == 0)
	{
		F_20ms = 1;
	}

	if(g_10ms_Count%5 == 0)
	{
		F_50ms = 1;
	}

	// 使用TC1生成500ms标志（50个10ms = 500ms）
	if(g_10ms_Count%50 == 0)
	{
		F_500ms = 1;
		g_10ms_Count=0;
	}
}

/**************************************************************************
函数名称：	void Light_RGB(void)
函数功能：	开机3色背光点亮各0.5s
输入参数：	无
输出参数：	无
返回值  ：	无
占用空间：	TBD
备    注：	20ms作为时基
**************************************************************************/
void Light_RGB(void)
{
	static bit F_First_Enter;

	g_20ms_Count ++;

	//首次点亮绿灯
	if( !F_First_Enter )
	{
		F_First_Enter = 1;
		g_20ms_Count = 0;
		LED_Green_En();
	}

#if Func_3color
	//0.5s后点亮黄灯
	if (g_20ms_Count == 25)
	{
		LED_Green_Dis();
		LED_Yellow_En();
	}

	//1s后点亮红灯
	if (g_20ms_Count == 50)
	{
		LED_Yellow_Dis();
		LED_Red_En();
	}

	//1.5s后关红灯
	if (g_20ms_Count == 75)
	{
		LED_Red_Dis();
		g_20ms_Count = 0;
		F_First_Enter = 0;
		F_LED_Enable = Disable;
	}
#else
	if (g_20ms_Count == 50)
	{
		LED_Green_Dis();
		g_20ms_Count = 0;
		F_First_Enter = 0;
		F_LED_Enable = Disable;
	}
#endif
}

/**************************************************************************
函数名称：	void Auto_TurnOff(void)
函数功能：	自动关机时间到，则进入Sleep模式关机
输入参数：	无
输出参数：	无
返回值  ：	无
占用空间：	TBD
备    注：	20ms作为时基
**************************************************************************/
void Auto_TurnOff(void)
{
	g_AutoTurnOff_Count--;

	if (g_AutoTurnOff_Count == 0)
	{
		//自动关机时保留生产/黑体模式，不重置为耳温
		if( eTestmode_num == Insptectmode || eTestmode_num == Blackbodymode )
		{
			g_PreserveSpecialMode = 1;
		}
		eMain_Task = Task_Sleepmode;
		eSleepTask_Sta = Sleep_false;
	}
}

/**************************************************************************
函数名称：	void Auto_TurnOff_Time_Sel(void)
函数功能：	自动关机时间选择
输入参数：	无
输出参数：	无
返回值  ：	无
占用空间：	TBD
备    注：	20ms作为时机，生产模式、黑体模式下6分钟自动关机，其他模式均为半分钟自动关机
**************************************************************************/
void Auto_TurnOff_Time_Sel(void)
{
	if (eTestmode_num == Insptectmode || eTestmode_num == Blackbodymode )
	{
		g_AutoTurnOff_Count = CountDown_6min;
	}
	else
	{
		g_AutoTurnOff_Count = CountDown_30s;
	}
}

/**************************************************************************
函数名称：	void Led_CountDown_3s(void)
函数功能：	背光15s倒计时
输入参数：	无
输出参数：	F_15s_TimeOut
返回值  ：	无
占用空间：	TBD
备    注：	20ms作为时基
**************************************************************************/
void Led_CountDown_15s(void)
{
	g_15s_Count --;
	if( !g_15s_Count )
	{
		LED_CloseAll();
	}
}



/**************************************************************************
函数名称：	uint8 Time_CountDown_5s_timeout(bit State)
函数功能：	准备就绪5s倒计时
输入参数：	State -> 0:运行程序    1：复位计数值
输出参数：	F_5s_TimeOut
返回值  ：	0:5秒倒计时还没结束   1：倒计时结束
占用空间：	TBD
备    注：	10ms作为时基
**************************************************************************/
uint8 Time_CountDown_5s_timeout(bit State)
{
    static uint8 a = 1;
    if(State)
    {
        a = 0;
        g_5s_Count = 0;
        return 0;
    }
    if(a == 0)
    {
        g_5s_Count = CountDown_5s;
        a = 1;
    }
    else
    {
        if(g_5s_Count == 0)
        {
			uSetFlag.bits.Ready_First=1;
            return 1;
        }
        else
        {
            g_5s_Count --;
        }
    }
    return 0;
}

/**************************************************************************
函数名称：	void Disp_Colon(void)
函数功能：	冒号闪烁
输入参数：	g_50ms_Count
输出参数：	冒号
返回值  ：	无
占用空间：	TBD
备    注：	500ms触发一次（50ms时基）
**************************************************************************/
void Disp_Colon(void)
{
	//当时间显示状态时冒号以0.5s频率闪烁
	if( F_Colon_Blink )
	{
		//lcd_colon_xor();	//冒号异或
	}
}



