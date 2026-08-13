/**************************************************************************
文件名称：	Drv_GPIO.c
说    明：	GPIO初始化函数集合（驱动层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
修订记录：
**************************************************************************/
#include "Include.h"

/**************************************************************************
函数名称：	Cal_Inspect_Detect()
函数功能：	校准模式、绑定模式入口检测
输入参数：	Port_Debug、Port_Cal
输出参数：	uSetFlag
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Cal_Inspect_Detect(void)
{
	//绑定检测模式、校温检测模式、系数调整模式判断
	if( !Port_Debug && Port_Cal )
	{
		if ( eTestmode_num == Insptectmode )
		{
			LED_CloseAll();
			eMain_Task = Task_ParamModifymode;	//生产模式短路debug口则进入系数调整模式
		}
		else
		{
			eSetTask = Set_TimeFormate;		//设置态恢复状态
			F_FirstEnter_SetMode = 0;
			eMain_Task = Task_BondTestmode;	//用户模式下短路debug进入绑定检测模式
		}
	}

	//设置态决不允许进入校温，汇编和C的程序结构不一样，未初始化adc
	if( !Port_Cal && !Port_Debug && eMain_Task == Task_ReadyMode )
	{
		eMain_Task = Task_Calimode;		//用户模式下短路debug和cal进入校温模式
	}
}

/**************************************************************************
函数名称：	CF_Check()
函数功能：	单位状态、单位可切换状态查询
输入参数：	Port_CF、Port_Change_CF
输出参数：	uSetFlag
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void CF_Check(void)
{
	//单位状态检测
	uSetFlag.bits.Unit = Unit_C;
	if( !Port_CF )
	{
		uSetFlag.bits.Unit = Unit_F;
	}

	//单位可切换状态检测
	uSetFlag.bits.Unit_Change = Unit_Change_En;
	if( !Port_Change_CF )
	{
		uSetFlag.bits.Unit_Change = Unit_Change_Dis;
	}
}

/**************************************************************************
函数名称：	GPIO_Init()
函数功能：	IO口初始化设置
输入参数：	P0、P1、P2、P3、P4、P5
输出参数：	P0、P1、P2、P3、P4、P5
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void GPIO_Init(void)
{
	//P00 P01 P04三个按键输入上拉高 ，P02 SDA输出不上拉高 ，P03 SCL输出不上拉高 ，P05 未使用输入不上拉低， P06/P07 XTI/XTO外部晶振未使用输入不上拉低
	//0: input mode, 1: output mode
	P0M = 0x0C;			//0b00001100
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P0UR = 0x13;		//0b00010011
	//0: low level, 1: high level
	// #if Nation
	// 	P0 = 0x33;		//0b00110011	//B版
	// #else
	P0 = 0x1F;			//0b00011111	//C版
	// #endif
	
	//P10 G320_EN电荷泵使能输出不上拉高， P11 BZ蜂鸣器输出不上拉低， P12 MOSI未使用输出不上拉低，P13 SCK输出不上拉低， P14 TX输入上拉高 ，P15 RX输出上拉高 ，P16 VOC_EN输出上拉高 ，P17 BLE_BUSY未使用输出上拉高
	//0: input mode, 1: output mode
	P1M = 0xEF;			//0b11101111
	//0: LCD functon pin, 1: IO pin
	P1SEG = 0xFF;		//均为普通IO口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P1UR = 0xF0;		//0b11110000
	//0: low level, 1: high level
	P1 = 0xF1;			//0b11110001

	//P23-P27为seg口输入不上拉低，耳套口P22输入上拉高，P20 BLE_EN未使用输入上拉高 ，P21 BLE_LINK 未使用输入上拉高
	//0: input mode, 1: output mode
	P2M = 0x00;			//0b00000000
	//0: LCD functon pin, 1: IO pin
	P2SEG = 0x07;		//0b00000111,P23-P27均为LCD口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P2UR = 0x07;		//0b00000111
	//0: low level, 1: high level
	P2 = 0x07;			//0b00000111
	
	//P50 MOTOR输出不上拉低，P51 Port_Change_CF输入上拉高， P52 Port_CF输入上拉高，P53 Port_Cal输入上拉高，P54 Port_Debug输入上拉高，P55 Green输出不上拉高 ，P56 Red输出不上拉高 ，P57 Yellow输出不上拉高
	//0: input mode, 1: output mode
	P5M = 0xE1;			//0b11100001
	//0: LCD functon pin, 1: IO pin
	P5SEG = 0xFF;		//0b11111111
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P5UR = 0x1E;		//0b00011110
	//0: low level, 1: high level
	P5 = 0xFE;			//0b11111110
	
	//P3均为LCD口输入不上拉低
	//0: input mode, 1: output mode
	P3M = 0x00;			//均为LCD口
	//0: LCD functon pin, 1: IO pin
	P3SEG = 0x00;		//均为LCD口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P3UR = 0x00;		//均为LCD口
	//0: low level, 1: high level
	P3 = 0x00;			//均为LCD口

	//P4均为LCD口输入不上拉低
	//0: input mode, 1: output mode
	P4M = 0x00;			//均为LCD口
	//0: LCD functon pin, 1: IO pin
	P4SEG = 0x00;		//均为LCD口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P4UR = 0x00;		//均为LCD口
	//0: low level, 1: high level
	P4 = 0x00;			//均为LCD口
}

// #if !Nation//国内用的时C版原理图，国外用的是B版
/**************************************************************************
函数名称：	GPIO_PowerDown()
函数功能：	IO口初始化设置
输入参数：	P0、P1、P2、P3、P4、P5
输出参数：	P0、P1、P2、P3、P4、P5
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void GPIO_PowerDown( void )		//C版
{
	//P0: 按键输入上拉, SDA/SCL输出0(I2C已关), 未用脚输出不上拉0
	//0: input mode, 1: output mode
	P0M = 0x2C;			//0b00101100 - P02,P03 output(SDA/SCL), 
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P0UR = 0x13;		//0b00010011 - P00,P01,P04按键上拉, P05不上拉
	//0: low level, 1: high level
	P0 = 0x1F;			//0b00011111 - SDA/SCL输出高
 
	//P1: 关闭所有使能脚，VOC_EN必须为1
	//0: input mode, 1: output mode
	P1M = 0xCF;			//0b11001111 - P14 input(TX), P15 output(I2C power)
	//0: LCD functon pin, 1: IO pin
	P1SEG = 0xFF;			//均为普通IO口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P1UR = 0x40;		//0b01000000 - 不需要上拉
	//0: low level, 1: high level
	P1 = 0x40;			//0b010000000 


	//P2: 耳套输出0
	//0: input mode, 1: output mode
	P2M = 0x07;			//0b00000111 
	//0: LCD functon pin, 1: IO pin
	P2SEG = 0x07;			//0b00000111,P23-P27均为LCD口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P2UR = 0x00;			//0b00000000 - P20,P21未用不上拉
	//0: low level, 1: high level
	P2 = 0x00;			//0b00000000


	//P5: MOTOR输出低, 检测脚输入上拉, 
	//P50 MOTOR ，P51 Port_Change_CF， P52 Port_CF ，P53 Port_Cal ，P54 Port_Debug，P55 Green ，P56 Red ，P57 Yellow
	//0: input mode, 1: output mode
	P5M = 0x07;			//0b00000111 
	//0: LCD functon pin, 1: IO pin
	P5SEG = 0xFF;			//0b11111111
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P5UR = 0xF8;			//0b11111000 
	//0: low level, 1: high level
	P5 = 0xF8;			//0b11111000 


	//0: input mode, 1: output mode
	P3M = 0x00;			//均为LCD口
	//0: LCD functon pin, 1: IO pin
	P3SEG = 0x00;		//均为LCD口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P3UR = 0x00;		//均为LCD口
	//0: low level, 1: high level
	P3 = 0x00;			//均为LCD口

	//0: input mode, 1: output mode
	P4M = 0x00;			//均为LCD口
	//0: LCD functon pin, 1: IO pin
	P4SEG = 0x00;		//均为LCD口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P4UR = 0x00;		//均为LCD口
	//0: low level, 1: high level
	P4 = 0x00;			//均为LCD口
}

// #else
/**************************************************************************
函数名称：	GPIO_PowerDown()
函数功能：	IO口初始化设置
输入参数：	P0、P1、P2、P3、P4、P5
输出参数：	P0、P1、P2、P3、P4、P5
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
#if 0
void GPIO_PowerDown( void ) //B版
{
	//SDA/SCL输出为0(因为采用的是硬体I2C),按键输入上拉高电平(耳套口P04要设为输出0),XIN/XOUT输入不上拉0
	//0: input mode, 1: output mode
	P0M = 0x1C;			//0b00011100
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P0UR = 0x23;		//0b00100011
	//0: low level, 1: high level
	P0 = 0x23;			//0b00100011

	//BZ不上拉输出0,MISO/MOSI/SCK/TX均输出不上拉低电平,RX为I2C供电口不上拉输出0,CE输出上拉高电平,PT17未使用输入上拉高
	//0: input mode, 1: output mode
	P1M = 0x7F;			//0b01111111
	//0: LCD functon pin, 1: IO pin
	P1SEG = 0xFF;		//均为普通IO口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P1UR = 0xC0;		//0b11000000
	//0: low level, 1: high level
	P1 = 0xC0;			//0b11000000

	// //BZ不上拉输出0,MISO/MOSI/SCK/TX均输出不上拉低电平,RX为I2C供电口不上拉输出0,CE输出上拉高电平,PT17未使用输入上拉高
	// //0: input mode, 1: output mode
	// P1M = 0xFF;			//0b11111111
	// //0: LCD functon pin, 1: IO pin
	// P1SEG = 0xFF;		//均为普通IO口
	// //0: disable pullup, 1: enable pullup, pull resistor = 200k
	// P1UR = 0x40;		//0b01000000
	// //0: low level, 1: high level
	// P1 = 0x40;			//0b01000000

	//P23-P27为seg口，其他均为输入上拉高电平
	//0: input mode, 1: output mode
	P2M = 0x00;			//0b00000000
	//0: LCD functon pin, 1: IO pin
	P2SEG = 0x07;		//0b00000111,P23-P27均为LCD口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P2UR = 0x07;		//0b00000111
	//0: low level, 1: high level
	P2 = 0x07;			//0b00000111

	//LVD输入上拉,CF/C/F输出不上拉0，CAL/Debug输入上拉高电平,RGB输入上拉高
	//0: input mode, 1: output mode
	P5M = 0x06;			//0b00000110
	//0: LCD functon pin, 1: IO pin
	P5SEG = 0xFF;		//0b11111111
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P5UR = 0xF9;		//0b11111001
	//0: low level, 1: high level
	P5 = 0xF9;			//0b11111001


	//0: input mode, 1: output mode
	P3M = 0x00;			//均为LCD口
	//0: LCD functon pin, 1: IO pin
	P3SEG = 0x00;		//均为LCD口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P3UR = 0x00;		//均为LCD口
	//0: low level, 1: high level
	P3 = 0x00;			//均为LCD口

	//0: input mode, 1: output mode
	P4M = 0x00;			//均为LCD口
	//0: LCD functon pin, 1: IO pin
	P4SEG = 0x00;		//均为LCD口
	//0: disable pullup, 1: enable pullup, pull resistor = 200k
	P4UR = 0x00;		//均为LCD口
	//0: low level, 1: high level
	P4 = 0x00;			//均为LCD口
}

#endif
