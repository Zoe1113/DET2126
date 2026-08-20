/**************************************************************************
文件名称：	Drv_LCD_ET05.c
说    明：	液晶显示函数集合（驱动层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
修订记录：
**************************************************************************/
#include "Include.h"

//只在本文件内使用的宏定义
#define	SA	8			//00000000 00001000
#define	SB	4			//00000000 00000100
#define	SC	2			//00000000 00000010
#define	SD	1			//00000000 00000001

#define	SE	512			//00000010 00000000
#define	SF	2048		//00001000 00000000
#define	SG	1024		//00000100 00000000
#define	P	256			//00000001 00000000

//常量定义
//后面应该用AbCdEF取代
uint16 __ROM	DispTable[10] =	 {	SA+SB+SC+SD+SE+SF,			//0		0
									SB+SC,						//1		1
									SA+SB+SD+SE+SG,				//2		2
									SA+SB+SC+SD+SG,				//3		3
									SB+SC+SF+SG,				//4		4
									SA+SC+SD+SF+SG,				//5		5
									SA+SC+SD+SE+SF+SG,			//6		6
									SA+SB+SC,					//7		7
									SA+SB+SC+SD+SE+SF+SG,		//8		8
									SA+SB+SC+SD+SF+SG,			//9		9
								} ;

/**************************************************************************
函数名称：	Lcd_Init()
函数功能：	Lcd初始化
输入参数：	无
输出参数：	无
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Lcd_Init( void )
{
	P4SEG = 0x00 ;
	P3SEG = 0X00 ;
	P2SEG = 0x07 ;			//设置LCD显示IO

	LCDM1 = 0x03 ;			//1/3Bias,C-Type LCD Mode.
	LCDM2 = 0x04 ;			//VLCD = 3.0V,
	LCDM3 = 0X02 ;			//Disable LCD low power mode./0 = 4-COM
	Delay50us(100) ;
	FLCDEN = 1 ;			//使能LCD
}

/**************************************************************************
函数名称：	Disp_Ntc()
函数功能：	显示环境温度
输入参数：	温度
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	环境温度依然是双精度,但显示单精度,且不进行四舍五入,直接舍弃最低位
**************************************************************************/

void Disp_Ntc(uint16 Temp)
{
	uint16 R_LCD1,R_LCD2,R_LCD3,R_LCD4;

	HexToBcd(Temp);

	//取千位
	R_LCD1 = Hex2Bcd[2] & 0x0F;
	//取百位
	R_LCD2 = Hex2Bcd[1] >> 4;
	//取十位
	R_LCD3 = Hex2Bcd[1] & 0x0F;
	//取个位
	R_LCD4 = Hex2Bcd[0] >> 4;

	//R_LCD1如果非0，不管R_LCD2是否为0都要显示，如果R_LCD1为0，那么R_LCD2非0才会显示
	if( R_LCD2 !=0 || R_LCD1!=0)
	{
		lcd6 = DispTable[ R_LCD2 ] >> 8;
    	lcd5 = DispTable[ R_LCD2 ];
	}
	lcd4 = DispTable[ R_LCD3 ] >> 8;
    lcd3 = DispTable[ R_LCD3 ];
	lcd2 = DispTable[ R_LCD4 ] >> 8;
    lcd1 = DispTable[ R_LCD4 ];

	//最高位置1
	if( R_LCD1 )
	{
		//显示1
		lcd_one_en(); 
	}

	//小数点必亮
	lcd_point_en();
}

/**************************************************************************
函数名称：	Disp_Temp()
函数功能：	显示测量值
输入参数：	point：0表示不显示小数点、1表示显示小数点
			High：0表示0.1显示模式，1表示0.01高精度模式
			Unit: 华氏＞=200判断标志位，此时High需置0
			temp：温度值（16进制）
输出参数：	LCD中188.88
返回值  ：	无
占用空间：	411words
备    注：	0.01精度可能需要在其他地方显示，视不同机型决定
			F华氏单位当>=200时需移位处理
			适合低精度显示、高精度显示、无小数点显示
**************************************************************************/
void Disp_Temp(bit Point, bit High, bit Unit, int16 Temp)
{
	uint16 R_LCD1,R_LCD2,R_LCD3,R_LCD4,R_LCD5;
	bit F_NegFlag = 0;

	//如果为负数转出正数，同时-20.00禁止显示高精度
	if ( Temp < 0 )
	{
		F_NegFlag = 1;
		Temp = ~Temp + 1;
		if (Temp == 2000)
		{
			Temp = Temp / 10;
			Point = 0;		//强行置零以防用户误输入
		}
	}

	//如果是华氏且大于199.99则禁止高精度显示
	if( Unit && (Temp > 0x4E1F) )
	{
		Temp = Temp / 10;
		Point = 0;		//强行置零以防用户误输入
	}

	//强制转换成无符号数
	Temp = (uint16)Temp;

	HexToBcd(Temp);

	//取百位
	R_LCD1 = Hex2Bcd[2] & 0x0F;
	//取十位
	R_LCD2 = Hex2Bcd[1] >> 4;
	//取个位
	R_LCD3 = Hex2Bcd[1] & 0x0F;
	//取0.1位
	R_LCD4 = Hex2Bcd[0] >> 4;
	//取0.01位
	R_LCD5 = Hex2Bcd[0] & 0x0F;

	if( High )
	{
		if( R_LCD3 !=0 || R_LCD2!=0)
		{
			lcd6 = DispTable[ R_LCD3 ] >> 8;
	    	lcd5 = DispTable[ R_LCD3 ];
		}
		else
		{
			lcd6 = 0x00;
			lcd5 = 0x00;
		}
		lcd4 = DispTable[ R_LCD4 ] >> 8;
	    lcd3 = DispTable[ R_LCD4 ];
		lcd2 = DispTable[ R_LCD5 ] >> 8;
	    lcd1 = DispTable[ R_LCD5 ];	
		//最高位置1
		if( R_LCD2>=4 )
		{
			lcd_one_en();
		}	
	}
	else
	{
		//R_LCD1如果非0，不管R_LCD2是否为0都要显示，如果R_LCD1为0，那么R_LCD2非0才会显示
		if( R_LCD2 !=0 || R_LCD1!=0)
		{
			lcd6 = DispTable[ R_LCD2 ] >> 8;
	    	lcd5 = DispTable[ R_LCD2 ];
		}
		else
		{
			lcd6 = 0x00;
			lcd5 = 0x00;
		}
		//对于温度来说8.8一定会显示的（当然要考虑校准态AD显示另算）
		lcd4 = DispTable[ R_LCD3 ] >> 8;
	    lcd3 = DispTable[ R_LCD3 ];
		lcd2 = DispTable[ R_LCD4 ] >> 8;
	    lcd1 = DispTable[ R_LCD4 ];
		//最高位置1
		if( R_LCD1 )
		{
			lcd_one_en();
		}
	}
	//小数点点亮
	if( Point )
	{
		lcd_point_en();
	}
}

/**************************************************************************
函数名称：	Disp_Code()
函数功能：	显示程序编码
输入参数：	程序编码（如142，则输入142）
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Code(uint16 num)
{
	uint16 R_LCD1,R_LCD2,R_LCD3;

	//取百位
	R_LCD1 = num / 100;
	//取十位
	num = num % 100;
	R_LCD2 = num / 10;
	//取个位
	R_LCD3 = num % 10;

	//对于程序编码888一定会显示的
	lcd6 = DispTable[ R_LCD1 ] >> 8;
    lcd5 = DispTable[ R_LCD1 ];
	lcd4 = DispTable[ R_LCD2 ] >> 8;
    lcd3 = DispTable[ R_LCD2 ];
	lcd2 = DispTable[ R_LCD3 ] >> 8;
    lcd1 = DispTable[ R_LCD3 ];
}

/**************************************************************************
函数名称：	Disp_Version()
函数功能：	显示程序版本
输入参数：	格式为U1.0，U和.为固定显示，10为输入的2位数字
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Version(uint16 num)
{
	uint16 R_LCD1,R_LCD2;

	//取十位
	R_LCD1 = num / 10;
	//取个位
	R_LCD2 = num % 10;

	lcd6 = 0x0A;
    lcd5 = 0x07;
	lcd4 = DispTable[ R_LCD1 ] >> 8;
    lcd3 = DispTable[ R_LCD1 ];
	
	lcd2 = DispTable[ R_LCD2 ] >> 8;
    lcd1 = DispTable[ R_LCD2 ];
	lcd_point_en();
}

/**************************************************************************
函数名称：	Clr_Disp()
函数功能：	完全清屏
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Clr_Disp(void)
{
	lcd0 = 0x00;
    lcd1 = 0x00;
    lcd2 = 0x00;
    lcd3 = 0x00;
    lcd4 = 0x00;
    lcd5 = 0x00;
    lcd6 =  0x00;
    lcd7 = 0x00;
    lcd8 = 0x00;
    lcd9 = 0x00;

}

/**************************************************************************
函数名称：	Clr_Disp_KeepBat()
函数功能：	清除LCD显示，但保留电池图标（lcd7的bit0电池外框/bit1电池内部）
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	用于需要在清屏后保留电池状态的场景（如记忆查看）
**************************************************************************/
void Clr_Disp_KeepBat(void)
{
	lcd0 = 0x00;
    lcd1 = 0x00;
    lcd2 = 0x00;
    lcd3 = 0x00;
    lcd4 = 0x00;
    lcd5 = 0x00;
    lcd6 =  0x00;
    lcd7 &= 0x03;	// 保留电池两位（bit0=外框, bit1=内部），清除lcd7其余位
    lcd8 = 0x00;
    lcd9 = 0x00;
}

/**************************************************************************
函数名称：	Clr_Disp888()
函数功能：	清除温度的3个888
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Clr_Disp888(void)
{
	lcd6 = 0x00;
    lcd5 = 0x00;
    lcd4 = 0x00;
    lcd3 = 0x00;
    lcd2 = 0x00;
    lcd1 = 0x00;
}

/**************************************************************************
函数名称：	Disp_All()
函数功能：	全显
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_All(void)
{
    lcd0 = 0xff;
    lcd1 = 0xff;
    lcd2 = 0xff;
    lcd3 = 0xff;
    lcd4 = 0xff;
    lcd5 = 0xff;
    lcd6 = 0xff;
    lcd7 = 0xff;
    lcd8 = 0xff;
    lcd9 = 0xff;
}

/**************************************************************************
函数名称：	Disp_Unit()
函数功能：	根据单位设置显示对应单位
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Unit(void)
{
	
	if( uSetFlag.bits.Unit )
	{
		lcd_unit_c_clr();	//清单位C
		lcd_unit_f_en();
	}
	else
	{
		lcd_unit_f_clr();	//清单位F
		lcd_unit_c_en();
	}
}

/**************************************************************************
函数名称：	Disp_SmileFace()
函数功能：	仅显示笑脸
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_SmileFace(void)
{
	lcd_badface_clr();	//清哭脸
	lcd_smileface_en();	//显笑脸
}

/**************************************************************************
函数名称：	void Disp_BadFace(void)
函数功能：	仅显示哭脸
输入参数：	无
输出参数：	LCD16
返回值  ：	无
占用空间：	TBD
备    注：	生产检验模式、黑体模式使用
**************************************************************************/
void Disp_BadFace(void)
{
	lcd_smileface_clr();	//清笑脸
	lcd_badface_en();	//显哭脸
}

/**************************************************************************
函数名称：	void Disp_ModeSign(void)
函数功能：	根据不同模式显示不同标志
输入参数：	无
输出参数：	LCD16
返回值  ：	无
占用空间：	TBD
备    注：	生产检验模式、黑体模式使用
**************************************************************************/
void Disp_ModeSign(void)
{
	lcd_ear_clr();		//清除耳温标志
	lcd_forehead_clr();	//清除额温标志
	lcd_obj_clr();		//清除物温标志
	//清除年龄分段标志
	lcd_person0_3_clr();
	lcd_person3_36_clr();
	lcd_person36_up_clr();
	switch ( eTestmode_num )
	{
		case Earmode:
			if(eMain_Task!=Task_Testingmode&&eReadyTask_Sta!=Ready_Timeout)
			{
				lcd_ear_en();
			}
			//耳温显示年龄分段
			lcd_person_en();	//显示轮廓
			switch(g_AgeGroup)
			{
				case AgeGroup_0_3:
					lcd_person0_3_en();
					break;
				case AgeGroup_3_36:
					lcd_person3_36_en();
					break;
				case AgeGroup_36_Plus:
				default:
					lcd_person36_up_en();
					break;
			}
			break;

		case Foreheadmode:
			lcd_forehead_en();
			//额温显示年龄分段
			lcd_person_en();	//显示轮廓
			switch(g_AgeGroup)
			{
				case AgeGroup_0_3:
					lcd_person0_3_en();
					break;
				case AgeGroup_3_36:
					lcd_person3_36_en();
					break;
				case AgeGroup_36_Plus:
				default:
					lcd_person36_up_en();
					break;
			}
			break;

		case Objectmode:
			//物温不显示年龄分段
			lcd_person_clr();
			lcd_obj_en();
			break;
		case Insptectmode:
			//生产模式: 测量时不显示模式符号，5s就绪后才显示
			lcd_person_clr();
			if(eMain_Task!=Task_Testingmode&&eReadyTask_Sta!=Ready_Timeout)
			{
				lcd_obj_en();
			}
			break;
		case Blackbodymode:
			//黑体模式: 测量时不显示模式符号，5s就绪后才显示
			if(eMain_Task!=Task_Testingmode&&eReadyTask_Sta!=Ready_Timeout)
			{
				lcd_obj_en();
			}
			//黑体显示年龄分段
			lcd_person_en();	//显示轮廓
			switch(g_AgeGroup)
			{
				case AgeGroup_0_3:
					lcd_person0_3_en();
					break;
				case AgeGroup_3_36:
					lcd_person3_36_en();
					break;
				case AgeGroup_36_Plus:
				default:
					lcd_person36_up_en();
					break;
			}
			break;
		default:
			break;
	}
}

/**************************************************************************
函数名称：	void Clr_ModeSign(void)
函数功能：	清除额温、耳温、物温标志
输入参数：	无
输出参数：	LCD16
返回值  ：	无
占用空间：	TBD
备    注：	生产检验模式、黑体模式使用
**************************************************************************/
void Clr_ModeSign(void)
{
	lcd_ear_clr();		//清除耳温标志
	lcd_forehead_clr();	//清除额温标志
	lcd_obj_clr();		//清除物温标志
}

/**************************************************************************
函数名称：	Disp_LowBat()
函数功能：	仅显示低电压符号
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_LowBat(void)
{
	lcd_fullbat_clr();
	lcd_bat_en();	//显示低电压符号；
	Disp_Lo();
	lcd_badface_clr();      //清笑脸
    lcd_smileface_clr();    //清哭脸
}

/**************************************************************************
函数名称：	Disp_FullBat()
函数功能：	仅显示满电压符号
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_FullBat(void)
{
	lcd_bat_en();
	lcd_fullbat_en();	//显示满电压符号；
}

/**************************************************************************
函数名称：	Disp_OFF()
函数功能：	显示Off
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_OFF(void)
{
	Clr_Disp();
	lcd6 = 0x0A;
    lcd5 = 0x0F;

    lcd4 = 0x0E;
    lcd3 = 0x08;

    lcd2 = 0x0E;
    lcd1 = 0x08;
}

/**************************************************************************
函数名称：	Disp_On()
函数功能：	显示On
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_On(void)
{
	Clr_Disp();
	lcd6 = 0x0A;
    lcd5 = 0x0F;

    lcd4 = 0x06;
    lcd3 = 0x02;
}

/**************************************************************************
函数名称：	Disp_Lo()
函数功能：	显示Lo
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Lo(void)
{
    lcd4 = 0x0A;
    lcd3 = 0x01;

    lcd2 = 0x06;
    lcd1 = 0x03;
	if(eTestmode_num !=Objectmode)
	{
		lcd_badface_clr();      //清笑脸
		lcd_smileface_en();    //清哭脸
	}
}

/**************************************************************************
函数名称：	Disp_Hi()
函数功能：	显示Hi
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Hi(void)
{
    lcd4 = 0x0E;
    lcd3 = 0x06;

    lcd2 = 0x02;
    lcd1 = 0x00;
	if(eTestmode_num !=Objectmode)
	{
		lcd_badface_en();      //清笑脸
		lcd_smileface_clr();    //清哭脸
	}
}

/**************************************************************************
函数名称：	Disp_Ready()
函数功能：	显示_ _ . _
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Ready(void)
{
    lcd6= 0x00;
    lcd5 = 0x01;
    lcd4= 0x00;
    lcd3 = 0x01;
    lcd2 = 0x00;
    lcd1 = 0x01;
	lcd_point_en();
	lcd_smileface_clr();	//清哭笑脸（从测试态退出）
	lcd_badface_clr();
    lcd_mem_clr();   //清M标志（从记忆态退出）
}


/**************************************************************************
函数名称：	Disp_FourSecLoop_Init() / Disp_FourSecLoop_Step()
函数功能：	Disp_FourSecLoop 的非阻塞版本。
			每 ~10ms 由主循环调用一次 Disp_FourSecLoop_Step()，内部按阶段推进动画。
			总时长 = g_5s_Count / 100 个周期，每周期 4 阶段 × 25 次调用 = 1 秒。
			返回 0=动画进行中，1=动画完成（5秒结束）。
输入参数：	无
输出参数：	LCD
返回值  ：	uint8 — 0：运行中 / 1：完成
占用空间：	TBD
备    注：	用于 Ready_DisEr1 状态，替代原阻塞版本，使 5 秒等待期间
			主循环仍可处理按键（SkeyProcess 等）。
**************************************************************************/
static uint8  nb_FourSecPhase = 0;   // 动画阶段 0-3
static uint8  nb_FourSecTick  = 0;   // 当前阶段内滴答计数
static uint8  nb_FourSecCycle = 0;   // 已完成周期数
static uint8  nb_FourSecTotal = 0;   // 总周期数

void Disp_FourSecLoop_Init(void)
{
	nb_FourSecPhase = 0;
	nb_FourSecTick  = 0;
	nb_FourSecCycle = 0;
	nb_FourSecTotal = g_5s_Count / 100;
	if(nb_FourSecTotal == 0) nb_FourSecTotal = 1;
}

uint8 Disp_FourSecLoop_Step(void)
{
	if(nb_FourSecCycle >= nb_FourSecTotal)
	{
		return 1;   // 动画完成
	}
		

	switch(nb_FourSecPhase)
	{
		case 0:   // 点亮 lcd6 横杠 + 耳温/物温图标
			if(nb_FourSecTick == 0)
			{
				Clr_Disp888();
				lcd_ear_clr();
				lcd_obj_clr();
				lcd6 = 0x04;
				lcd5 = 0x00;
			if(eTestmode_num == Insptectmode || eTestmode_num == Blackbodymode)
			{
				lcd_obj_en();
			}
			else if(eTestmode_num == Earmode)
			{
				lcd_ear_en();
			}
			}
			if(++nb_FourSecTick >= 25)   // 250ms / 10ms = 25 次
			{
				nb_FourSecTick = 0;
				nb_FourSecPhase = 1;
			}
			break;

		case 1:   // 点亮 lcd4 横杠，清除图标
			if(nb_FourSecTick == 0)
			{
				lcd4 = 0x04;
				lcd3 = 0x00;
			if(eTestmode_num == Insptectmode || eTestmode_num == Blackbodymode)
			{
				lcd_obj_clr();
			}
			else if(eTestmode_num == Earmode)
			{
				lcd_ear_clr();
			}
			}
			if(++nb_FourSecTick >= 25)
			{
				nb_FourSecTick = 0;
				nb_FourSecPhase = 2;
			}
			break;

		case 2:   // 点亮 lcd2 横杠
			if(nb_FourSecTick == 0)
			{
				lcd2 = 0x04;
				lcd1 = 0x00;
			}
			if(++nb_FourSecTick >= 25)
			{
				nb_FourSecTick = 0;
				nb_FourSecPhase = 3;
			}
			break;

		case 3:   // 清屏
			if(nb_FourSecTick == 0)
			{
				Clr_Disp888();
				lcd_ear_clr();
				lcd_obj_clr();
			}
			if(++nb_FourSecTick >= 25)
			{
				nb_FourSecTick = 0;
				nb_FourSecPhase = 0;
				nb_FourSecCycle++;
			}
			break;
	}

	return (nb_FourSecCycle >= nb_FourSecTotal) ? 1 : 0;
}

/**************************************************************************
函数名称：	Disp_Null()
函数功能：	记忆模式显示- - -
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Null(void)
{
    Clr_Disp_KeepBat();
    lcd6 = 0x04;
    lcd5 = 0x00;
    lcd4 = 0x04;
    lcd3 = 0x00;
    lcd2 = 0x04;
    lcd1 = 0x00;
    lcd_mem_en();  //显示M标志
}
/**************************************************************************
函数名称：	Disp_ErN()
函数功能：	显示错误代码ErN，n可以为0-9的值
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_ErN(uint8 num)
{
	lcd6 = 0x0E;
    lcd5 = 0x09;
    lcd4 = 0x06;
    lcd3 = 0x00;
	lcd2 = DispTable[ num ] >> 8;
    lcd1 = DispTable[ num ];
}

/**************************************************************************
函数名称：	void Disp_ErrMsg(uint8 ErrNum)
函数功能：	显示错误菜单
输入参数：	uErrFlag.g_ErrFlag
输出参数：	LCD
返回值  ：	无
占用空间：	28 words
备    注：	错误存在优先级（Er2>Lo/Hi>Er3/Er4），优先级可以在本程序定义，也可以有根据错误发生的先后决定
**************************************************************************/
void Disp_ErrMsg(void)
{
	switch (uErrFlag.g_ErrFlag)
	{
		case 2:
			lcd_badface_clr();
			lcd_smileface_clr();
			Disp_ErN(2);
			break;
		case 4:
			lcd_badface_clr();
			lcd_smileface_en();
			Disp_ErN(3);
			break;
		case 8:
			lcd_badface_clr();
			lcd_smileface_clr();
			Disp_ErN(4);
			break;
		case 0x10:
			lcd_badface_clr();
			lcd_smileface_clr();
			Disp_ErN(5);
			break;
		case 0x20:	
			lcd_badface_clr();
			lcd_smileface_clr();
			Disp_ErN(6);
			break;
		case 0x40:
			lcd_badface_clr();
			lcd_smileface_clr();
			Disp_Lo();
			break;
		case 0x80:
			lcd_badface_clr();
			lcd_smileface_clr();
			Disp_Hi();
			break;
		default:
			break;
	}
	if(eTestmode_num == Insptectmode)
	{
		lcd_one_clr();			//清除温度前的1图标(lcd6 bit0)，保留数码管段
		lcd_person36_up_clr();	//清除36月+年龄图标(lcd7 bit3)，保留电池图标
	}
}

/**************************************************************************
函数名称：	Disp_NtcEr2()
函数功能：	显示环温错误代码Er2
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_NtcEr2(void)
{
    Disp_ErN(2);
}

/**************************************************************************
函数名称：	Disp_CAL()
函数功能：	黑体模式显示CAL
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_CAL(void)
{
	Clr_Disp();
	lcd6 = 0x0A;
    lcd5 = 0x09;
    lcd4 = 0x0E;
    lcd3 = 0x0E;
    lcd2 = 0x0A;
    lcd1 = 0x01;
}

/**************************************************************************
函数名称：	Disp_Ab()
函数功能：	调试模式显示Ab
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Ab(void)
{
	Clr_Disp();
	lcd4 = 0x0E;
    lcd3 = 0x0E;
    lcd2 = 0x0E;
    lcd1 = 0x03;
}

/**************************************************************************
函数名称：	Disp_PAS()
函数功能：	调试模式显示PAS
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_PAS(void)
{
	Clr_Disp();
	lcd6 = 0x0E;
    lcd5 = 0x0C;
    lcd4 = 0x0E;
    lcd3 = 0x0E;
    lcd2 = 0x0C;
    lcd1 = 0x0B;
}

/**************************************************************************
函数名称：	Disp_Err()
函数功能：	调试模式显示Err
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Err(void)
{
	lcd6 = 0x0E;
    lcd5 = 0x09;
    lcd4 = 0x06;
    lcd3 = 0x00;
    lcd2 = 0x06;
    lcd1 = 0x00;
}

/**************************************************************************
函数名称：	Disp_Debug1()
函数功能：	绑定检测模式显示测试画面1
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Debug1(void)
{
	Clr_Disp();
	lcd6 = 0x06;
    lcd5 = 0x0A;
    lcd4 = 0x06;
    lcd3 = 0x0A;
    lcd2 = 0x06;
    lcd1 = 0x0A;
	lcd_voice_en();
	lcd_bat_en();
	lcd_fullbat_en();
	lcd_mem_en();
	lcd_person_en();
	lcd_person36_up_en();
	lcd_badface_en();
	lcd_smileface_en();
	lcd_mem_en();
	lcd_obj_en();
	lcd_unit_f_en();
}

/**************************************************************************
函数名称：	Disp_Debug2()
函数功能：	绑定检测模式显示测试画面2
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Debug2(void)
{
	Clr_Disp();
	lcd6 = 0x09;
    lcd5 = 0x05;
    lcd4 = 0x08;
    lcd3 = 0x05;
    lcd2 = 0x09;
    lcd1 = 0x05;
	lcd_person0_3_en();
	lcd_person3_36_en();
	lcd_unit_c_en();
	lcd_ear_en();
	lcd_forehead_en();
}

/**************************************************************************
函数名称：	void Disp_DebugPASn(uint8 num)
函数功能：	绑定检测模式显示PASn(n为：0-9)
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_DebugPASn(uint8 num)
{
    lcd6 = 0x0E;
    lcd5 = 0x0C;
    lcd4 = 0x0E;
    lcd3 = 0x0E;
	lcd2= DispTable[ num ] >> 8;
    lcd1 = DispTable[ num ];
}

/**************************************************************************
函数名称：	Disp_12H()
函数功能：	设置态显示12H
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_12H(void)
{
	Clr_Disp();
    lcd0 = 0x00;
    lcd1 = 0x06;
    lcd2 = 0x06;
    lcd3 = 0x0B;
	lcd4 = 0x07;
	lcd5 = 0x06;
}

/**************************************************************************
函数名称：	Disp_24H()
函数功能：	设置态显示24H
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_24H(void)
{
	Clr_Disp();
    lcd0 = 0x06;
    lcd1 = 0x0B;
    lcd2 = 0x03;
    lcd3 = 0x06;
	lcd4 = 0x07;
	lcd5 = 0x06;
}

/**************************************************************************
函数名称：	Disp_Ch()
函数功能：	设置态显示CH
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
#if Soft_Code != 339 &&  Soft_Code != 477
void Disp_Ch(void)
{
	Clr_Disp();
    lcd4 = 0x0A;
    lcd3 = 0x09;
    lcd2 = 0x0E;
    lcd1 = 0x06;
}
#endif

/**************************************************************************
函数名称：	Disp_En()
函数功能：	设置态显示EN
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_En(void)
{
  	Clr_Disp();
    lcd4 = 0x0E;
    lcd3 = 0x09;
    lcd2 = 0x06;
    lcd1 = 0x02;
}

/**************************************************************************
函数名称：	Disp_SP()
函数功能：	设置态显示SP,西班牙语
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
#if Soft_Code == 339 || Soft_Code == 477
void Disp_SP(void)
{
	Clr_Disp();
    lcd4 = 0x0C;
    lcd3 = 0x0B;
    lcd2 = 0x0E;
    lcd1 = 0x0C;
}
#endif


/**************************************************************************
函数名称：	Disp_Table1()
函数功能：	显示Table1
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Table1(void)
{
	Clr_Disp_KeepBat();
    lcd4 = 0x0E;
    lcd3 = 0x01;
    lcd2 = 0x00;
    lcd1 = 0x06;
}

/**************************************************************************
函数名称：	Disp_Table2()
函数功能：	显示Table2
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_Table2(void)
{
	Clr_Disp_KeepBat();
    lcd4 = 0x0E;
    lcd3 = 0x01;
    lcd2 = 0x06;
    lcd1 = 0x0D;
}

/**************************************************************************
函数名称：	Disp_VoiceSign()
函数功能：	显示喇叭符号
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Disp_VoiceSign(void)
{

	if( uSetFlag.bits.Voiceflag==1 )
	{
		lcd_voice_en();
		// lcd_voice_volumn_en();		
	}  
	else
	{
		lcd_voice_clr();
		// lcd_voice_volumn_clr();		
	}
	
}

/**************************************************************************
函数名称：	void LVD_Display(void)
函数功能：	低电时显示
输入参数：	void
输出参数：	void
返回值  ：	无
**************************************************************************/
void LVD_Display(void)
{
	if(uStaFlag.bits.midBat != 1 && uStaFlag.bits.LowBat != 1)
	{
		lcd_bat_en();
		lcd_fullbat_en();

	}
	else if(uStaFlag.bits.LowBat != 1 && uStaFlag.bits.midBat == 1)
	{
		lcd_bat_xor();
		lcd_fullbat_clr();
	}
	else if(uStaFlag.bits.LowBat == 1)
	{
		lcd_bat_en();
		lcd_fullbat_clr();
	}
}

//额温加载动画状态
uint8 g_ForeheadLoading_Sta = 0;
uint8 g_ForeheadLoading_Timer = 0;

/**************************************************************************
函数名称：	Disp_ForeheadLoading_Init()
函数功能：	初始化额温测量加载显示动画
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	调用此函数开始非阻塞动画
**************************************************************************/
void Disp_ForeheadLoading_Init(void)
{
	g_ForeheadLoading_Sta = 1;
	g_ForeheadLoading_Timer = 35;
	Clr_Disp888();
}

/**************************************************************************
函数名称：	Disp_ForeheadLoading_Process()
函数功能：	额温测量加载显示动画处理（非阻塞）
输入参数：	无
输出参数：	LCD
返回值  ：	bit 1-动画进行中，0-动画结束
占用空间：	TBD
备    注：	需要每隔10ms调用一次，与数据采集并行执行
**************************************************************************/
bit Disp_ForeheadLoading_Process(void)
{
	if(g_ForeheadLoading_Sta == 0)
		return 0;
	
	if(++g_ForeheadLoading_Timer >= 35)	//每300ms切换一帧
	{
		g_ForeheadLoading_Timer = 0;
		g_ForeheadLoading_Sta++;
		
		switch(g_ForeheadLoading_Sta)
		{
			case 2:
				lcd3 = 0x01;
				break;
			case 3:
				lcd4 = 0x02;
				break;
			case 4:
				lcd4 = 0x0A;
				break;
			case 5:
				lcd3 = 0x09;
				break;
			case 6:
				lcd1 = 0x08;
				break;
			case 7:
				lcd1 = 0x0C;
				break;
			case 8:
				lcd1 = 0x0E;
				break;
			case 9:
				lcd1 = 0x0F;
				break;
			case 10:
				Clr_Disp888();
				g_ForeheadLoading_Sta = 0;
				break;
			default:
				g_ForeheadLoading_Sta = 0;
				break;
		}
	}
	return (g_ForeheadLoading_Sta != 0);
}

/**************************************************************************
函数名称：	Disp_ForeheadLoading_Stop()
函数功能：	停止额温测量加载显示动画
输入参数：	无
输出参数：	LCD
返回值  ：	无
占用空间：	TBD
备    注：	立即停止动画并清屏
**************************************************************************/
void Disp_ForeheadLoading_Stop(void)
{
	g_ForeheadLoading_Sta = 0;
	g_ForeheadLoading_Timer = 0;
	Clr_Disp888();
}






