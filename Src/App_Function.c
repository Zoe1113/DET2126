/**************************************************************************
文件名称：	App_Function.c
说    明：	功能类函数集合（应用层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
修订记录：
**************************************************************************/
#include "Include.h"

//只在本文件内使用的宏定义
//温度分段阈值
#define SlightTempC	0x0E92		//37.3
#define FeverTempC	0x0EC4		//37.8
#define SlightTempF	0x26B6		//99.1
#define FeverTempF	0x2710		//100.0

//不同年龄分段的正常体温上限(摄氏度*100)
#define NormalTemp_0_3M_C       0x0E9C    //37.4°C (0-3月)
#define NormalTemp_3_36M_C      0x0EB0    //37.6°C (3-36月)
#define NormalTemp_36M_Plus_C   0x0EBA    //37.7°C (36月+)

//不同年龄分段的低烧阈值(摄氏度*100)
#define LowFeverTemp_3_36M_C    0x0F0A    //38.5°C (3-36月)
#define LowFeverTemp_36M_Plus_C 0x0F64    //39.4°C (36月+)

//不同年龄分段的正常体温上限(华氏度*100)
#define NormalTemp_0_3M_F       0x26D4    //99.4°F (0-3月)
#define NormalTemp_3_36M_F      0x26E8    //99.6°F (3-36月)
#define NormalTemp_36M_Plus_F   0x2706    //99.9°F (36月+)

//不同年龄分段的低烧阈值(华氏度*100)
#define LowFeverTemp_3_36M_F    0x2792    //101.3°F (3-36月)
#define LowFeverTemp_36M_Plus_F 0x283C    //103.0°F (36月+)

//体温过低阈值
#define LowTemp_C               0x0D48    //34.0°C
#define LowTemp_F               0x2468    //93.2°F

//体温过高阈值
#define HighTemp_C              0x10CC    //43.0°C
#define HighTemp_F              0x2ABC    //109.4°F

uint8 Hex2Bcd[3];	//hex转bcd后bcd码存储区

/**************************************************************************
函数名称：	void Tone_Init(void)
函数功能：	发音程序
输入参数：	无
输出参数：	无
返回值  ：	无
占用空间：	TBD
备    注：	提高解耦，减少程序占用，提高对主体程序的影响
**************************************************************************/
void Tone_Init(void)
{
    if(  uSetFlag.bits.Voiceflag == 1 )
    {
        #if Have_Voice_Func
			AM5BA_Send_Cmd(LL_Stop, SS_Stop, Nodata);
			Delay10ms(1);
			AM5BA_Send_Cmd(LL_Broadcast, SS_Di, Nodata);
			Delay10ms(1);
        #else
            BZ_Beep230();
        #endif
    }
    else
    {
        #if Have_Motor
            if( eTestmode_num != Earmode )
                g_MotorSystick = 25;
        #endif
    }                  		
}

// 报警模式常量
#define ALARM_PATTERN_SINGLE	0	// 单次: 1x230ms
#define ALARM_PATTERN_LO		1	// 低温: 4x160ms
#define ALARM_PATTERN_FEVER		2	// 发烧: 230ms + 3x160ms

/**************************************************************************
函数名称：	void Alarm_BeepVib(uint8 pattern)
函数功能：	蜂鸣器与振动报警辅助函数
输入参数：	pattern (ALARM_PATTERN_SINGLE/LO/FEVER)
输出参数：	无
返回值  ：	无
备    注：	从Fever_alarm中提取的公共报警模式
**************************************************************************/
void Alarm_BeepVib(uint8 pattern)
{
	if(uSetFlag.bits.Voiceflag == 1 && eMain_Task != Task_Memorymode)
	{
		switch(pattern)
		{
			case ALARM_PATTERN_SINGLE:
				BZ_Beep230();
				break;
			case ALARM_PATTERN_LO:
				BZ_Beep160();
				BZ_Beep160();
				BZ_Beep160();
				BZ_Beep160();
				break;
			case ALARM_PATTERN_FEVER:
				BZ_Beep230();
				BZ_Beep160();
				BZ_Beep160();
				BZ_Beep160();
				break;
		}
	}
	if(uSetFlag.bits.Motorflag == 1 && eMain_Task != Task_Memorymode)
	{
		switch(pattern)
		{
			case ALARM_PATTERN_SINGLE:
				MT_Vib230();
				break;
			case ALARM_PATTERN_LO:
				MT_Vib160();
				MT_Vib160();
				MT_Vib160();
				MT_Vib160();
				break;
			case ALARM_PATTERN_FEVER:
				MT_Vib230();
				MT_Vib160();
				MT_Vib160();
				MT_Vib160();
				break;
		}
	}
}

/**************************************************************************
函数名称：	void Fever_alarm(int16 Temp)
函数功能：	发烧报警判断子程序(优化版)
输入参数：	Temp (0.1精度温度值)
输出参数：	无
返回值  ：	无
占用空间：	约300~500 words (原版1107 words)
备    注：	必须在摄氏华氏转化之后，且调整为0.1分辨率后使用
			优化: Lo/Hi提前返回; 报警模式提取为Alarm_BeepVib; 阈值选择与动作逻辑分离
**************************************************************************/
void Fever_alarm(int16 Temp)
{
	int16 lowTemp, highTemp, normalUpper, yellowUpper;
	uint8 hasYellow;

	LED_CloseAll();
	uStaFlag.bits.Fever = 0;

	// 选择单位相关的Lo/Hi阈值
	if(uSetFlag.bits.Unit)
	{
		lowTemp = (int16)LowTemp_F;
		highTemp = (int16)HighTemp_F;
	}
	else
	{
		lowTemp = (int16)LowTemp_C;
		highTemp = (int16)HighTemp_C;
	}

	// Lo: 体温过低 - 所有年龄段完全相同
	if(Temp < lowTemp)
	{
		LED_Green_En();
		Disp_SmileFace();
		uErrFlag.bits.Lo = 1;
		Alarm_BeepVib(ALARM_PATTERN_LO);
		return;
	}

	// Hi: 体温过高 - 所有年龄段完全相同
	if(Temp > highTemp)
	{
		LED_Red_En();
		Disp_BadFace();
		uErrFlag.bits.Hi = 1;
		Alarm_BeepVib(ALARM_PATTERN_FEVER);
		return;
	}

	// 选择年龄段相关的Normal/Yellow阈值
	if(uSetFlag.bits.Unit)
	{
		switch(g_AgeGroup)
		{
			case AgeGroup_0_3:
				normalUpper = (int16)NormalTemp_0_3M_F;
				yellowUpper = highTemp;	// 0-3月无黄色区间
				hasYellow = 0;
				break;
			case AgeGroup_3_36:
				normalUpper = (int16)NormalTemp_3_36M_F;
				yellowUpper = (int16)LowFeverTemp_3_36M_F;
				hasYellow = 1;
				break;
			case AgeGroup_36_Plus:
				normalUpper = (int16)NormalTemp_36M_Plus_F;
				yellowUpper = (int16)LowFeverTemp_36M_Plus_F;
				hasYellow = 1;
				break;
			default:
				return;
		}
	}
	else
	{
		switch(g_AgeGroup)
		{
			case AgeGroup_0_3:
				normalUpper = (int16)NormalTemp_0_3M_C;
				yellowUpper = highTemp;
				hasYellow = 0;
				break;
			case AgeGroup_3_36:
				normalUpper = (int16)NormalTemp_3_36M_C;
				yellowUpper = (int16)LowFeverTemp_3_36M_C;
				hasYellow = 1;
				break;
			case AgeGroup_36_Plus:
				normalUpper = (int16)NormalTemp_36M_Plus_C;
				yellowUpper = (int16)LowFeverTemp_36M_Plus_C;
				hasYellow = 1;
				break;
			default:
				return;
		}
	}

	// 正常温度: 绿灯+笑脸+单次报警
	if(Temp <= normalUpper)
	{
		LED_Green_En();
		Disp_SmileFace();
		Alarm_BeepVib(ALARM_PATTERN_SINGLE);
	}
	// 低烧(黄色区间): 黄灯+笑脸+单次报警
	else if(hasYellow && Temp <= yellowUpper)
	{
		LED_Yellow_En();
		Disp_SmileFace();
		Alarm_BeepVib(ALARM_PATTERN_SINGLE);
	}
	// 高烧: 红灯+哭脸+发烧报警
	else
	{
		LED_Red_En();
		Disp_BadFace();
		uStaFlag.bits.Fever = 1;
		Alarm_BeepVib(ALARM_PATTERN_FEVER);
	}
}
/**************************************************************************
函数名称：	uint8 LeapYear_Judge( uint16 L_Buf )
函数功能：	闰年判断子程序
输入参数：	L_Buf（年份）
输出参数：	无
返回值  ：	1：闰年，0：非闰年
占用空间：	TBD
备    注：
;闰年条件:	a、能被4整除,不能被100整除的年份是闰年; b、能被400整除的年份是闰年
;技巧：能100整除，那么十进制的个位和十位一定为0，能被4整除，那么二进制的低2位一定也为0
**************************************************************************/
uint8 LeapYear_Judge( uint16 L_Buf )
{
	HexToBcd(L_Buf);
	if( Hex2Bcd[0] == 0 )
	{
		if( (L_Buf & 0x000F) == 0)
			return 1;
		else
			return 0;
	}
	else
	{
		if( (L_Buf & 0x0003) == 0)
			return 1;
		else
			return 0;
	}
}

/*************************************************************************************************************
函数名称：	int16 CToF(uint16 L_Temp)
函数功能：	摄氏转华氏子程序
输入参数：	g_TpStep
输出参数：	g_TpStep
返回值  ：	无
占用空间：	TBD
备    注：	计算公式:摄氏度转华氏度公式 : F = C*1.8 +32 转换成华氏度只要小数点后两位，第三位直接舍弃
**************************************************************************************************************/
int16 CToF(int16 L_Temp)
{
	if( uSetFlag.bits.Unit )
	{
		if( L_Temp < 0 )
		{   
            //计算方法：F =  32 - C*2 + C*0.2
			L_Temp = labs(L_Temp);
			L_Temp = L_Temp << 1;
			L_Temp = L_Temp - L_Temp/10;
			L_Temp = 0x0C80 - L_Temp;
		}
		else
		{
            //计算方法：F = C*2 - C*0.2 +32
			L_Temp = L_Temp << 1; //C*2
			L_Temp = L_Temp - (L_Temp + 9)/10; //(L_Temp + 9)/10是C*0.2,加9是为了保证小数点第二位数值正确
			L_Temp += 0x0C80;
		}
	}
	return L_Temp;
}

/**************************************************************************
函数名称：	int16 Temp_Resolution_Adjust(int16 L_Temp)
函数功能：	显示分辨率调整子程序
输入参数：	L_Temp（0.01精度）
输出参数：	L_Temp（0.1精度）
返回值  ：	L_Temp（0.1精度）
占用空间：	TBD
备    注：	无
**************************************************************************/
int16 Temp_Resolution_Adjust(int16 L_Temp)
{
	bit F_NegFlag = 0;

	if( L_Temp < 0)
	{
		L_Temp = ~L_Temp + 1;
		F_NegFlag = 1;
	}

	L_Temp += 5;
	L_Temp = L_Temp/10;
	L_Temp = L_Temp*10;

	if (F_NegFlag)
	{
		L_Temp = ~L_Temp + 1;
	}
	return L_Temp;
}

/**************************************************************************
函数名称：	void Temp_Relate(void)
函数功能：	显示分辨率调整子程序
输入参数：	g_TpStep、g_RelateTemp
输出参数：	g_RelateTemp
返回值  ：	无
占用空间：	TBD
备    注：	前后两次温度差值在0.5℃内,则求前后两次的平均值作为此次的显示值，超出则显示当前值
**************************************************************************/
void Temp_Relate(void)
{
	int16 a;
    a = g_TpStep - g_RelateTemp;
	a = labs(a);
	if ( a < 0x0033 )
    {
     	g_TpStep += g_RelateTemp;
       	g_TpStep = g_TpStep >> 1;
    }
	g_RelateTemp = g_TpStep;
}

/**************************************************************************
函数名称：	Emissivity_correction()
函数功能：	发射率调整
输入参数：	g_TpCount、g_Emission
输出参数：	g_TpCount
返回值  ：	无
占用空间：	TBD
备    注：	TPCount=TPCountH*系数/100
原理:		其实并不是发射率调整,因为发射率应该是进行除法操作,这里的目的是降低TP值以使温度与口腔等效而乘以某个特定的系数
**************************************************************************/
void Emissivity_correction(void)
{
	g_TpCount = (int32)g_TpCount * g_Emission / 100;
}

/**************************************************************************
函数名称：	void Body_MeasureRange_Check(void)
函数功能：	人体测量范围判断子程序，判断是否超出34-43℃
输入参数：	g_TpStep
输出参数：	uErrFlag.bits.Lo，uStaFlag.bits.Hi
返回值  ：	无
占用空间：	TBD
备    注：	本程序修改后必须在Temp_correction子程序后面使用
**************************************************************************/
void Body_MeasureRange_Check(void)
{
	int16 L_TpStep;

	uErrFlag.g_ErrFlag = 0;

	// 使用与 Fever_alarm 一致的四舍五入精度(0.1°C)，避免精度不一致导致
	// 显示"Lo"但报警为正常温度的Bug（如33.95~33.99°C区间）
	L_TpStep = Temp_Resolution_Adjust(g_TpStep);

	if( L_TpStep < DispRange_HumanDown )
	{
		uErrFlag.bits.Lo = 1;
	}

	if( L_TpStep > DispRange_HumanUp )
	{
		uErrFlag.bits.Hi = 1;
	}
}

/**************************************************************************
函数名称：	void Obj_MeasureRange_Check(void)
函数功能：	物体测量范围判断子程序，判断是否超出0-100℃
输入参数：	g_TpStep
输出参数：	uErrFlag.bits.Lo，uErrFlag.bits.Hi
返回值  ：	无
占用空间：	TBD
备    注：	本程序修改后必须在Temp_correction子程序后面使用
;带有物温模式的产品,本程序是可以只有物温模式进入,黑体态直接使用人体范围(Body_MeasureRange_Check)会有问题,因为范围不一致,故需警惕显示范围测试方法和后果.
;但无物温模式的产品,建议黑体模式只有人体范围,但黑体态直接使用人体范围(Body_MeasureRange_Check)会有问题,因为范围不一致,故需警惕显示范围测试方法和后果.
;需注意法规中显示范围和测量范围概念的区别,所以黑体态下使用人体范围是可以的(即使不一致,范围要广)
**************************************************************************/
void Obj_MeasureRange_Check(void)
{
	int16 L_TpStep;
	uErrFlag.g_ErrFlag = 0;
	L_TpStep = Temp_Resolution_Adjust(g_TpStep);

	if( L_TpStep < (int16)DispRange_ObjDown )
	{
		uErrFlag.bits.Lo = 1;
	}

	if( L_TpStep > DispRange_ObjUp )
	{
		uErrFlag.bits.Hi = 1;
	}
}

/**************************************************************************
函数名称：	void NtcTableWider_Check(void)
函数功能：	热敏电阻温度范围判断子程序性，判断是否超出0-50℃
输入参数：	g_NtcStep
输出参数：	uErrFlag.bits.Er2
返回值  ：	无
占用空间：	TBD
备    注：	本函数绝对不可以增加uErrFlag.g_ErrFlag = 0;的动作，如果超出范围，则NtcTable_Check也一定超出
**************************************************************************/
void NtcTableWider_Check(void)
{
	uErrFlag.bits.Er2 = 0;
	//IDE实测7FFF，考虑adc90%良性区域，故7FFF*0.9=0x7333;
	if( g_AIN2Count > 0x7333 || g_AIN1Count > 0x7333 || g_NtcCount < g_NtcTableUp2 || g_NtcCount > g_NtcTableDown2 )
	{
		uErrFlag.g_ErrFlag = 0;
	    uErrFlag.bits.Er2 = 1;
	}
}

/**************************************************************************
函数名称：	void NtcTable_Check(void)
函数功能：	热敏电阻温度范围判断子程序性，判断是否超出10-40℃
输入参数：	g_NtcStep
输出参数：	uErrFlag.bits.Er2
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void NtcTable_Check(void)
{
	uErrFlag.bits.Er2 = 0;
	//IDE实测7FFF，考虑adc90%良性区域，故7FFF*0.9=0x7333;

	//g_NtcCount = 0x1980; //35℃
	//g_NtcCount = 0x3D5C; //15℃
	//g_NtcCount = 0x1980-1;
	//g_NtcCount = 0x3D5C+1; 

	//NtcCount = 0x14CF; //40℃
	//NtcCount = 0x4D97;   //10℃
	//g_NtcCount = 0x14CF-1; 
	//NtcCount = 0x4D97+1;   
	if( g_AIN2Count > 0x7333 || g_AIN1Count > 0x7333 || g_NtcCount < g_NtcTableUp1 || g_NtcCount > g_NtcTableDown1 )
	{
		uErrFlag.g_ErrFlag = 0;
	    uErrFlag.bits.Er2 = 1;
	}
}

/**************************************************************************
函数名称：	void HexToBcd(uint16 Hex_Value)
函数功能：	2字节16进制转10进制程序
输入参数：	Hex_Value（2字节16进制）
输出参数：	LCD
返回值  ：	无
占用空间：	0.4ms（Fcpu=2M）  114Byte
备    注：	2字节转十进制使用该程序不会节省程序空间，建议使用/和%，2字节以上可以使用
**************************************************************************/
void HexToBcd(uint16 Hex_Value)
{
	uint8 k0,k1,loopnum;

	k0 = Hex_Value;
	Hex_Value = Hex_Value >> 8;
	k1 = Hex_Value;
	__asm{
			SelectBank(Hex2Bcd)
    		CLR	_Hex2Bcd+2
		  	CLR	_Hex2Bcd+1
		  	CLR	_Hex2Bcd
			MOV A,#16
			SelectBank(loopnum)
			MOV	CNameToAsmLabel(loopnum),A
		LOOPHEX:
			B0BCLR	FC

			SelectBank(k0)
			RLCM	CNameToAsmLabel(k0)
			SelectBank(k1)
			RLCM	CNameToAsmLabel(k1)

			SelectBank(Hex2Bcd)
			MOV	A,_Hex2Bcd
			ADC	A,_Hex2Bcd
			DAA
		 	MOV	_Hex2Bcd,A
			MOV	A,_Hex2Bcd+1
			ADC A,_Hex2Bcd+1
			DAA
			MOV	_Hex2Bcd+1,A
			MOV	A,_Hex2Bcd+2
			ADC	A,_Hex2Bcd+2
			DAA
			MOV	_Hex2Bcd+2,A
			SelectBank(loopnum)
			DECMS	CNameToAsmLabel(loopnum)
			JMP	LOOPHEX
		 }
}