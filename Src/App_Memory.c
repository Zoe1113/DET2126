/**************************************************************************
文件名称：	App_Memory.c
说    明：	记忆模块（应用层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
修订记录：
**************************************************************************/
#include "Include.h"

//记忆显示的几种方式
//1、记忆只显示测量值（不全部显示--- 或 全部清空，只显示第一页）---以上选一种
//2、10页全部显示，无记忆时显示---，此方法只在A/D/H模式
//3、记忆只显示测量值，有记忆时显示最后一页为最旧的记忆值，按上翻不循环，给用户提示什么？

//只在本文件内使用的宏定义
#if !Memory_Mode
	#define MemMaxArray 0x0A	//记忆最大数量
#else
	#define MemMaxArray 0x1E	//记忆最大数量
#endif

//变量定义
volatile bit F_MemNull;		//记忆为空标志位，1为空，0为有
volatile bit F_Mem_FirstEnter;	//首次进入记忆模式标志位
static bit F_MemNo_Disp;	//记忆序号显示标志位
static bit F_Mem_Disp;		//记忆值显示标志位

static volatile uint8 g_MemNo;			//记忆地址记录号
static volatile uint8 g_MemTotalNo;		//记忆总记录数
static volatile uint8 g_MemCount;		//记忆循环计数器

uint8 g_MonthMem;		//在显示记忆模式显示温度时的月份缓冲区

#if Memory_Mode
uint8 g_MonthMem_momeory;  //记忆模式下的模式信息
uint8 g_AgeGroup_Mem;
#endif
/**************************************************************************
函数名称：	App_Memory()
函数功能：	记忆功能处理
输入参数：	无
输出参数：	记忆序号、记忆值
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void App_Memory(void)
{
	int16 L_Temp;
	TKeyProcess();
	SkeyProcess();
	//App_PCKeyProcess();
	//首次进入
	if(F_Mem_FirstEnter==0)
	{
		F_Mem_FirstEnter = 1;	//设置已进入标志位
		F_MemNo_Disp = 0;		//首次进入不显示序号
		Mem_Init();				//初始化记忆模块
	}

	//按键按下，且未显示记忆序号，显示记录序号
	if ( uKeyPress.bits.MemKeyPress && !F_MemNo_Disp )
	{
		F_MemNo_Disp = 1;		//设置显示刷新标志位
		F_Mem_Disp = 0;
		Disp_MemNo();
		LED_CloseAll();
		LED_Green_En();
		if( uSetFlag.bits.Voiceflag )
		{
			BZ_Beep230();
		}
		if( uSetFlag.bits.Motorflag == 1 )
		{
			MT_Vib230();			
		}
		g_15s_Count = CountDown_15s;		//记忆显示15s倒计时
		Auto_TurnOff_Time_Sel();	//自动关机时间清零
	}

	//按键抬起，且未显示记忆值，显示记录值
	if ( uKeyRelease.bits.MemKeyRelease && !F_Mem_Disp )
	{
		uKeyRelease.bits.MemKeyRelease = 0;
		F_MemNo_Disp = 0;		//清除序号显示刷新标志位
		F_Mem_Disp = 1;
		L_Temp = Disp_Mem();
		if( L_Temp )
		{
			#if !Memory_Mode
				if ( eTestmode_num == Earmode || eTestmode_num == Foreheadmode)
			#else
				if( g_MonthMem_momeory & 0x40 || g_MonthMem_momeory & 0x20 ) 
			#endif
				{
					//保存当前年龄分段，使用记忆中的年龄分段进行判断
					eAgeGroup L_SaveAgeGroup = g_AgeGroup;
					#if Memory_Mode
						g_AgeGroup = (eAgeGroup)g_AgeGroup_Mem;
					#endif
					Fever_alarm(L_Temp);
					//恢复当前年龄分段
					g_AgeGroup = L_SaveAgeGroup;
				}
			else
			{
				LED_CloseAll();
				LED_Green_En();
			}
			g_15s_Count = CountDown_15s;		//记忆显示15s倒计时
		}
	}
}

/**************************************************************************
函数名称：	Mem_Init()
函数功能：	记忆初始化处理
输入参数：	无
输出参数：	F_MemNull是否为空，g_MemCount计数器
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
static void Mem_Init(void)
{
	//读取总记录数和当前数
	Read_Mem_RecordNo();

	//记忆显示的几种方式，全为空时，只显示第一页---
	g_MemCount = 0;		//记忆循环计数器
	F_MemNull = 1;		//默认记忆为空
	if( g_MemTotalNo )
	{
		F_MemNull = 0;	//R_MemTotalNo不为0表示不为空，有记忆
		g_MemCount = g_MemTotalNo;
	}
}

/**************************************************************************
函数名称：	Disp_MemNo()
函数功能：	显示记忆序号，记忆为空时显示相应提示
输入参数：	g_MemCount
输出参数：	LCD显示
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
static void Disp_MemNo(void)
{
	uint8 i,j;

	//记忆显示的几种方式，全为空时，只显示---
	if(F_MemNull)
	{
		Disp_Null();
		if(eTestmode_num ==Insptectmode)
		{
			lcd_unit_c_en();
		}
		//Disp_ModeSign();
		//清除年龄分段标志
		lcd_person_clr();
		lcd_person0_3_clr();
		lcd_person3_36_clr();
		lcd_person36_up_clr();

		Disp_VoiceSign();//显示语音标志
		#if Have_Voice_Func
			PlayStatueParam(2 , 0 , 0);
			PlayStatueParam(1 , Play_Stop,0);
			PlayStatueParam(1 , Play_NoMem,0);
		#endif
	}
	else
	{
		if( g_MemCount == 0 )
		{
			g_MemCount = g_MemTotalNo;
		}
		g_MemCount--;
		i = g_MemTotalNo - g_MemCount;
        Clr_Disp_KeepBat();
		Disp_VoiceSign();//显示语音标志
		// lcd_badface_clr();	//清哭脸
		// lcd_smileface_clr();	//清笑脸
		// lcd_unit_c_clr();
		// lcd_unit_f_clr();
		j = i/10;
		if( j )
		{
			lcd4 = DispTable[ j ] >> 8;
			lcd3 = DispTable[ j ];
		}
		j = i%10;
		lcd2 = DispTable[ j ] >> 8;
		lcd1 = DispTable[ j ];
		if(eTestmode_num == Insptectmode)
		{
			lcd6 = 0x00;
			lcd5 = 0x00;
		}
		lcd_mem_en();	//M标志

		#if !Memory_Mode
			Disp_ModeSign();
		#else
			lcd_ear_clr();		//清除耳温标志
			lcd_forehead_clr();	//清除额温标志
			lcd_obj_clr();		//清除物温标志
		#endif

		#if Have_Voice_Func
		if(eTestmode_num != Insptectmode)
		{
			PlayStatueParam(2 , 0 , 0);
			PlayStatueParam(1 , Play_Stop,0);
			g_DiDo = i;
			PlayStatueParam(1 , Play_MemNum,0);
		}
		#endif

		}
	}

/**************************************************************************
函数名称：	static int16 Disp_Mem(void)
函数功能：	显示记忆值
输入参数：	Mem_EarAdd、Mem_ForeAdd、Mem_ObjAdd
输出参数：	LCD显示值
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
static int16 Disp_Mem(void)
{
	int16 L_Temp = 0;
	uint8 L_MemAdd;

	if(!F_MemNull)
	{
		//获取记忆地址
		L_MemAdd = Read_Mem_Address(g_MemNo);

		//读取温度
		I2C_masterInit();
		L_Temp = I2C_Random_R(L_MemAdd);
		L_Temp = L_Temp << 8;
		L_MemAdd++;
		L_Temp |= I2C_Random_R(L_MemAdd);
		//读取模式标志位（复用原月份存储位置）
		L_MemAdd++;
		g_MonthMem = I2C_Random_R(L_MemAdd);
		//读取年龄分段
		L_MemAdd++;
		g_AgeGroup_Mem = I2C_Random_R(L_MemAdd);
		I2C_Disable();

		#if Memory_Mode
			g_MonthMem_momeory = g_MonthMem;
			g_MonthMem &= 0x0F;
		#endif

		//显示记忆值

		L_Temp = CToF(L_Temp);
		if( eTestmode_num == Insptectmode )
			Disp_Temp(0,1,uSetFlag.bits.Unit,L_Temp);	//显示温度
		else
		{
			L_Temp = Temp_Resolution_Adjust(L_Temp);
			Disp_Temp(1,0,uSetFlag.bits.Unit,L_Temp);	//显示温度
		}
		lcd_mem_en();	//M标志

		#if !Memory_Mode
			Disp_ModeSign();	//显示模式标志
		#else
			if( g_MonthMem_momeory & 0x40 )        //耳温
			{
				lcd_obj_clr();		//清除物温标志
				lcd_forehead_clr(); //清除额温标志
				lcd_ear_en();	    //显示耳温标志
				//显示年龄分段
				lcd_person_en();
				switch(g_AgeGroup_Mem)
				{
					case AgeGroup_0_3:
						lcd_person0_3_en();
						lcd_person3_36_clr();
						lcd_person36_up_clr();
						break;
					case AgeGroup_3_36:
						lcd_person3_36_en();
						lcd_person0_3_clr();
						lcd_person36_up_clr();
						break;
					case AgeGroup_36_Plus:
					default:
						lcd_person36_up_en();
						lcd_person0_3_clr();
						lcd_person3_36_clr();
						break;
				}
			}
			else if(g_MonthMem_momeory & 0x20)    //额温
			{   

				lcd_obj_clr();		//清除物温标志
				lcd_ear_clr();	    //清除耳温标志
				lcd_forehead_en();  //显示额温标志	
				//显示年龄分段
				lcd_person_en();
				switch(g_AgeGroup_Mem)
				{
					case AgeGroup_0_3:
						lcd_person0_3_en();
						lcd_person3_36_clr();
						lcd_person36_up_clr();
						break;
					case AgeGroup_3_36:
						lcd_person3_36_en();
						lcd_person0_3_clr();
						lcd_person36_up_clr();
						break;
					case AgeGroup_36_Plus:
					default:
						lcd_person36_up_en();
						lcd_person0_3_clr();
						lcd_person3_36_clr();
						break;
				}		
			}
			else                                      //物温标志
			{
				lcd_ear_clr();	    //清除耳温标志
				lcd_forehead_clr(); //清除额温标志
				lcd_obj_en();       //显示物温标志	
				//物温不显示年龄分段
				lcd_person_clr();
				lcd_person0_3_clr();
				lcd_person3_36_clr();
				lcd_person36_up_clr();		
			}
		#endif
		Disp_Unit();

		#if Have_Voice_Func
		if(eTestmode_num != Insptectmode)
		{
			g_DiDo = (uint16)L_Temp;
			PlayStatueParam(1 , Play_MemTemp,0);
		}
		#endif

		//指向下一个查询地址
		g_MemNo --;
		if( g_MemNo == 0 )
		{
			//记忆总记录数小于10时，表示总记录数和当前记录数相等，循环设为10
			if( g_MemTotalNo < MemMaxArray )
			{
				g_MemNo	= g_MemTotalNo;
			}
			else
			{
				g_MemNo = MemMaxArray;
			}
		}
	}
	return L_Temp;
}

/**************************************************************************
函数名称：	Mem_Store()
函数功能：	记忆值存储
输入参数：	L_Temp、g_MemTotalNo、g_MemNo
输出参数：	EEPROM
返回值  ：	无
占用空间：	TBD
备    注：	存储时间和日期
**************************************************************************/
void Mem_Store(int16 L_Temp)
{
	uint8 L_MemAdd;
	#if Memory_Mode
		uint8 L_tempbuf;
	#endif

	Mem_Init();
	if ( g_MemTotalNo < MemMaxArray )
	{
		g_MemTotalNo ++;
		g_MemNo	++;
	}
	else
	{
		g_MemNo ++;
		if ( g_MemNo == (g_MemTotalNo + 1) )
		{
			g_MemNo = 0x01;
		}
	}

	//获取总记录数和当前数在EEPROM中的地址
	#if !Memory_Mode
		switch (eTestmode_num)
		{
			case Earmode:
				L_MemAdd = 	I2C_Add_EarMem;
				break;
			case Foreheadmode:
				L_MemAdd = 	I2C_Add_ForeMem;
				break;
			case Objectmode:
				L_MemAdd = 	I2C_Add_ObjMem;
				break;
			//其他测试模式按obj存储处理
			default:
				L_MemAdd = 	I2C_Add_ObjMem;
				break;
		}
	#else
		L_tempbuf = g_Month;
	    switch (eTestmode_num)
		{
			case Earmode:
				L_tempbuf = g_Month | 0x40;
				break;
			case Foreheadmode:
				L_tempbuf = g_Month  | 0x20;
			break;
			/* case Blackbodymode:
				L_tempbuf = g_Month | 0x10;
				break; */
			case Objectmode:
				L_tempbuf = g_Month;
				break;
			//其他测试模式按obj存储处理
			default:
				L_tempbuf = g_Month;
				break;
		}
		L_MemAdd = 	I2C_Add_EarMem;
	#endif

	I2C_masterInit();

	//存储总记录数和当前记录数
	I2C_Byte_W(L_MemAdd, g_MemTotalNo);
	Delay1ms(5);
	L_MemAdd = L_MemAdd + I2C_Add_Offset;
	I2C_Byte_W(L_MemAdd, g_MemNo);
	Delay1ms(5);

	//获取记忆地址
	L_MemAdd = Read_Mem_Address(g_MemNo);

	//存储温度
	I2C_Byte_W(L_MemAdd, L_Temp >> 8);
	Delay1ms(5);
	L_MemAdd ++;
	I2C_Byte_W(L_MemAdd, L_Temp);
	Delay1ms(5);
	//存储模式标志位（复用原月份存储位置）
	L_MemAdd++;

	#if !Memory_Mode
		I2C_Byte_W(L_MemAdd, g_Month);
	#else
		I2C_Byte_W(L_MemAdd, L_tempbuf);
	#endif
	Delay1ms(5);
	
	//存储年龄分段
	L_MemAdd++;
	I2C_Byte_W(L_MemAdd, (uint8)g_AgeGroup);
	Delay1ms(5);
	I2C_Disable();
}

/**************************************************************************
函数名称：	void Read_Mem_RecordNo(void)
函数功能：	读取记忆记录数
输入参数：	无
输出参数：	g_MemTotalNo、g_MemNo
返回值  ：	无
占用空间：	TBD
备    注：
**************************************************************************/
void Read_Mem_RecordNo(void)
{
	uint8 L_MemAdd;

	//获取总记录数和当前数在EEPROM中的地址
	#if !Memory_Mode
		switch (eTestmode_num)
		{
			case Earmode:
				L_MemAdd = 	I2C_Add_EarMem;
				break;
			case Foreheadmode:
				L_MemAdd = 	I2C_Add_ForeMem;
				break;
			case Objectmode:
				L_MemAdd = 	I2C_Add_ObjMem;
				break;
			//其他测试模式按obj存储处理
			default:
				L_MemAdd = 	I2C_Add_ObjMem;
				break;
		}
	#else
		L_MemAdd = 	I2C_Add_EarMem;
	#endif

	I2C_masterInit();
	g_MemTotalNo = I2C_Random_R(L_MemAdd);		//读取总记录数
	g_MemNo = I2C_Random_R(L_MemAdd + I2C_Add_Offset);		//读取当前数
	I2C_Disable();
}

/**************************************************************************
函数名称：	uint8 Read_Mem_Address(uint8 L_MemNo)
函数功能：	读取记忆地址
输入参数：	L_MemNo记忆序号
输出参数：	L_MemAdd记忆地址值
返回值  ：	无
占用空间：	TBD
备    注：
**************************************************************************/
uint8 Read_Mem_Address(uint8 L_MemNo)
{
	uint8 L_MemAdd;

	#if !Memory_Mode
		//根据不同模式选择对应的EEPROM地址
		switch (eTestmode_num)
		{
			case Earmode:
				L_MemAdd = 	Mem_EarAdd;
				break;
			case Foreheadmode:
				L_MemAdd = 	Mem_ForeAdd;
				break;
			case Objectmode:
				L_MemAdd = 	Mem_ObjAdd;
				break;
			//其他测试模式按obj存储处理
			default:
				L_MemAdd = 	Mem_ObjAdd;
				break;
		}
	#else
		L_MemAdd = 	Mem_EarAdd;
	#endif

	//计算地址：温度2字节 + 模式标志位1字节+年龄分段1字节，共4字节
	L_MemAdd += (L_MemNo - 1) * 4;
	return L_MemAdd;
}

/**************************************************************************
函数名称：	void Clr_All_Memory(void)
函数功能：	清除全部记忆
输入参数：	无
输出参数：	无
返回值  ：	无
占用空间：	TBD
备    注：
**************************************************************************/
void Clr_All_Memory(void)
{
	I2C_masterInit();
	I2C_Byte_W(I2C_Add_EarMem, 0x00);		//清除耳温总记录数
	Delay1ms(5);

	#if !Memory_Mode
	I2C_Byte_W(I2C_Add_ForeMem, 0x00);		//清除额温总记录数
	Delay1ms(5);

	I2C_Byte_W(I2C_Add_ObjMem, 0x00);		//清除物温总记录数
	Delay1ms(5);
	#endif

	I2C_Byte_W(I2C_Add_EarMem + I2C_Add_Offset, 0x00);	//清除耳温当前记录数
	Delay1ms(5);

	#if !Memory_Mode
	I2C_Byte_W(I2C_Add_ForeMem + I2C_Add_Offset, 0x00);	//清除额温当前记录数
	Delay1ms(5);

	I2C_Byte_W(I2C_Add_ObjMem + I2C_Add_Offset, 0x00);	//清除物温当前记录数
	Delay1ms(5);
	#endif

	I2C_Disable();
}