/**************************************************************************
文件名称：	App_SetMode.c
说    明：	设置模式（应用层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
备    注:
修订记录：
**************************************************************************/
#include "Include.h"

#define left 0
#define right 1

uint8 g_Blink;	//闪烁
uint8 g_Blink_count;	//闪烁时间(10ms时基为准)

eSetModeTask eSetTask;
eParamModifyTask eParamTask;			//参数调整模式当前子任务

bit F_UpdateValue;		//更新设置值
bit F_UpdateMenu;		//更新设置菜单
bit F_FirstEnter_SetMode;	//首次进入设置模式
bit F_FirstEnter_ParamModifymode;	//首次进入参数调整模式
uint8 L_MaxDay;

void App_SetMode(void)
{
	//初始化测试键为短按（短按+加速按功能）
	HalKey_Set_KeyMode(Func_Short_Continue, &sTestKey);
	//初始化设置键为短按
	HalKey_Set_KeyMode(Func_Short, &sSetKey);

	//校准检测判断，检测校准状态(只有在校准模式状态下，才可以进入并打开全部功能)
	Cal_Inspect_Detect();

	if(eMain_Task == Task_Unitmode)
	{
		static uint8 L_UnitHoldCnt = 0;	//长按计数
		//长按循环切换单位
		if(uKeyHold.bits.TKeyHold)
		{
			L_UnitHoldCnt++;
			if(L_UnitHoldCnt >= 25)	//约400ms切换一次
			{
				L_UnitHoldCnt = 0;
				uSetFlag.bits.Unit = ~uSetFlag.bits.Unit;
				Auto_TurnOff_Time_Sel();	//按下关机时间清0
			}
		}
		else if(uKeyPress.bits.TKeyPress)
		{
			L_UnitHoldCnt = 0;
			uKeyPress.bits.TKeyPress = 0;
			uSetFlag.bits.Unit = ~uSetFlag.bits.Unit;
			Auto_TurnOff_Time_Sel();	//按下关机时间清0
		}
		else
		{
			L_UnitHoldCnt = 0;
		}
		Disp_Unit();
		if(uKeyPress.bits.SKeyPress&&!uKeyHold.bits.SKeyHold)
		{
			eMain_Task = Task_Sleepmode;
			eSleepTask_Sta = Sleep_false;
			if(uSetFlag.bits.Voiceflag ==1)
			{
				BZ_Beep230();
			}
			if( uSetFlag.bits.Motorflag == 1 )
			{
				MT_Vib230();			
			}
		}
	}
	else if(eMain_Task == Task_ParamModifymode)
	{
		App_ParamModifymode();
	}
}

/**************************************************************************
函数名称：	void App_ParamModifymode(void)
函数功能：	参数调整模式（应用层）
			1、首次进入初始化按键和显示
			2、设置键短按：切换参数项
			3、测试键短按：参数值+1
			4、测试键长按：参数值连续递增
			5、循环到 Param_End 时再次按设置键，保存并退出到关机
输入参数：	无
输出参数：	无
返回值  ：	无
占用空间：	TBD
备    注：	参考 Src-代码 工程 App_SetMode.c 的 SetMode 参数项分支
**************************************************************************/
void App_ParamModifymode(void)
{
	bit L_UpdateValue = 0;	//本次循环是否需要更新参数值
	bit L_UpdateMenu = 0;	//本次循环是否需要更新显示

	//首次进入初始化
	if( !F_FirstEnter_ParamModifymode )
	{
		if(uSetFlag.bits.Voiceflag ==1)
		{
			BZ_Beep230();
		}
		if( uSetFlag.bits.Motorflag == 1 )
		{
			MT_Vib230();			
		}
		F_FirstEnter_ParamModifymode = 1;
		//设置键短按（用于切换参数项）
		HalKey_Set_KeyMode(Func_Short, &sSetKey);
		//测试键短按+加速按（短按调整一次，长按可连续加速调整）
		HalKey_Set_KeyMode(Func_Short_Continue, &sTestKey);
		HalKey_KeyClr();	//清除所有按键信息，防止刚进入就误触
		Auto_TurnOff_Time_Sel();	//重置自动关机计时

		eParamTask = Param_Emission;	//首次进入从发射率开始
		L_UpdateMenu = 1;
	}

	//设置键短按：切换参数项
	if( uKeyPress.bits.SKeyPress && !uKeyHold.bits.SKeyHold )
	{
		uKeyPress.bits.SKeyPress = 0;
		Auto_TurnOff_Time_Sel();	//按下自动关机时间清0
		L_UpdateMenu = 1;
		eParamTask ++;
		//循环到 Param_End 时再按一次：保存并退出
		if( eParamTask >= Param_End )
		{
			if(uSetFlag.bits.Voiceflag ==1)
			{
				BZ_Beep230();
			}
			if( uSetFlag.bits.Motorflag == 1 )
			{
				MT_Vib230();			
			}
			//保存到 EEPROM
			I2C_masterInit();
			I2C_Byte_W(I2C_Add_Emission, g_Emission);
			Delay1ms(5);
			I2C_Byte_W(I2C_Add_HumanRatio, g_HumanRatio);
			Delay1ms(5);
			//表格序号：0x10 地址只取 bit0
			I2C_Byte_W(I2C_Add_Table, uStaFlag.bits.TableNum);
			Delay1ms(5);
			I2C_Disable();

			//恢复标志位，保证下次进入首先初始化
			F_FirstEnter_ParamModifymode = 0;
			//进入关机
			eMain_Task = Task_Sleepmode;
			eSleepTask_Sta = Sleep_false;
			return;
		}
	}

	//测试键短按：参数值+1（按下立即触发）
	if( uKeyPress.bits.TKeyPress )
	{
		uKeyPress.bits.TKeyPress = 0;
		L_UpdateValue = 1;
	}

	//测试键长按/加速按：参数值连续递增
	if( uKeyHold.bits.TKeyHold )
	{
		uKeyHold.bits.TKeyHold = 0;
		L_UpdateValue = 1;
	}
	if( uKeyContinue.bits.TKeyContinue )
	{
		uKeyContinue.bits.TKeyContinue = 0;
		L_UpdateValue = 1;
	}

	//参数值调整
	if( L_UpdateValue )
	{
		Auto_TurnOff_Time_Sel();	//按下自动关机时间清0
		L_UpdateMenu = 1;			//修改参数后必须立即刷新显示
		switch( eParamTask )
		{
			//发射率：96~100（1.00），循环
			case Param_Emission:
				g_Emission ++;
				if( g_Emission > 100 )
					g_Emission = 96;
				break;

			//人体系数：0~60，循环
			case Param_HumanRatio:
				g_HumanRatio ++;
				if( g_HumanRatio > 60 )
					g_HumanRatio = 0;
				break;

			//表格选项：0/1 切换
			case Param_TableNum:
				uStaFlag.bits.TableNum = ~uStaFlag.bits.TableNum;
				break;

			default:
				break;
		}
	}

	//显示更新
	if( L_UpdateMenu )
	{
		//用 Clr_Disp_KeepBat 避免打乱 LVD 闪烁节奏
		Clr_Disp_KeepBat();
		switch( eParamTask )
		{
			//发射率：100 表示 1.00，显示 x.xx 三位小数
			case Param_Emission:
				Disp_Temp(0, 0, 0, (uint16)g_Emission * 10);
				break;

			//人体系数：60 表示 0.60，显示 x.xx 三位小数
			case Param_HumanRatio:
				Disp_Temp(0, 0, 0, (uint16)g_HumanRatio * 10);
				break;

			//表格选项：显示 Table1 / Table2 字符
			case Param_TableNum:
				if( uStaFlag.bits.TableNum )
					Disp_Table2();
				else
					Disp_Table1();
				break;

			default:
				break;
		}
	}
}

