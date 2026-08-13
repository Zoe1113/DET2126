/**************************************************************************
文件名称：	App_Param.c
说    明：	系统参数设置函数集合（应用层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
适用机型：
备	  注：	本程序将所有参数都预设定（包括语音、耳套、耳温等参数）
修订记录：
**************************************************************************/
#include "Include.h"

//变量定义
uint8 g_HumanRatio;		//人体系数
uint8 g_Emission;		//发射率
uint8 g_PcRatio;		//耳套修正
uint8 g_CheckSum;		//CRC校验位

/**************************************************************************
函数名称：	void Param_Init(void)
函数功能：	默认参数设置
输入参数：	无
输出参数：	无
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Param_Init(void)
{
	I2C_masterInit();
	g_CheckSum = 0x00;

	I2C_Byte_W(I2C_Add_Cali25TP, 0x00);
	Delay1ms(5);

	I2C_Byte_W(I2C_Add_Cali25TP + 1, 0x00);			//R_Cali25TP
	Delay1ms(5);

    g_CheckSum += g_Table37C >> 8;
	I2C_Byte_W(I2C_Add_Cali37TP, g_Table37C >> 8);
	Delay1ms(5);

    g_CheckSum += g_Table37C;
	I2C_Byte_W(I2C_Add_Cali37TP + 1, g_Table37C);	//R_Cali37Data
	Delay1ms(5);

    g_CheckSum += g_Table41C >> 8;
	I2C_Byte_W(I2C_Add_Cali41TP, g_Table41C >> 8);
	Delay1ms(5);

    g_CheckSum += g_Table41C;
	I2C_Byte_W(I2C_Add_Cali41TP + 1, g_Table41C);	//R_Cali41Data
	Delay1ms(5);

	//额温所有型号必须定义Distence_En，耳温不需要
	#if ET_FT ==1
	#if Distence_En
		g_CheckSum +=Distance_5cmval >> 8;
		I2C_Byte_W(Distance_5cm, Distance_5cmval >> 8);
		Delay1ms(5);
		g_CheckSum += Distance_5cmval;
		I2C_Byte_W(Distance_5cm + 1, Distance_5cmval);
		Delay1ms(5);
	#endif	
	#endif

	I2C_Byte_W(I2C_Add_Table, 0x00);		//黑体修正表格
	Delay1ms(5);


	//仅带语音款适用
	#if Have_Voice_Func
		#if Voice_Lang
			I2C_Byte_W(I2C_Add_Voice, lang_En);		//语音款需要（默认英文）
			Delay1ms(5);
		#else
			I2C_Byte_W(I2C_Add_Voice, lang_Ch);		//语音款需要（默认英文）
			Delay1ms(5);
		#endif
	#endif

#if ET_FT !=1
	I2C_Byte_W(I2C_Add_Emission, 0x64);		//发射率：1.00（仅耳温需要）
	Delay1ms(5);

	I2C_Byte_W(I2C_Add_HumanRatio, HumanRatio_Num);	//人体系数：0.02（仅耳温需要）
	Delay1ms(5);

	I2C_Byte_W(I2C_Add_PcRatio, 0x37);		//耳套系数：0.55（仅耳温需要）
	Delay1ms(5);

	I2C_Byte_W(I2C_Add_PcStatus, 0x00);		//耳套状态：080H：有耳套；00H：无耳套（仅ETA需要）
	Delay1ms(5);
#endif

	I2C_Byte_W(I2C_Add_EarMem, 0x00);		//存耳温记忆总记录数
	Delay1ms(5);

#if ET_FT !=1
	I2C_Byte_W(I2C_Add_ForeMem, 0x00);		//存额温记忆总记录数
	Delay1ms(5);

	I2C_Byte_W(I2C_Add_ObjMem, 0x00);		//存物温记忆总记录数
	Delay1ms(5);
#endif

	I2C_Byte_W(I2C_Add_EarMem + I2C_Add_Offset, 0x00);	//存耳温记忆记录号
	Delay1ms(5);

#if ET_FT !=1
	I2C_Byte_W(I2C_Add_ForeMem + I2C_Add_Offset, 0x00);	//存额温记忆记录号
	Delay1ms(5);

	I2C_Byte_W(I2C_Add_ObjMem + I2C_Add_Offset, 0x00);	//存物温记忆记录号
	Delay1ms(5);
#endif

	I2C_Byte_W(I2C_Add_CheckSum, g_CheckSum);		//存校验和(校验和一定识别码之前写入)
	Delay1ms(5);

	I2C_Byte_W(I2C_Add_IdentifyCode, IdentifyCode);		//识别码
	Delay1ms(5);

	I2C_Disable();
}

/**************************************************************************
函数名称：	Param_Check()
函数功能：	校准参数校验+读取其他参数
输入参数：	无
输出参数：	各变量
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Param_Check(void)
{
	I2C_masterInit();
	g_CheckSum = 0x00;

    g_Cali25TP = I2C_Random_R(I2C_Add_Cali25TP);
    g_CheckSum += g_Cali25TP;
	Delay1ms(5);

	g_Cali25TP = (g_Cali25TP << 8) | I2C_Random_R(I2C_Add_Cali25TP+1);
    g_CheckSum += g_Cali25TP;
	Delay1ms(5);

    g_Cali37Data = I2C_Random_R(I2C_Add_Cali37TP);
    g_CheckSum += g_Cali37Data;
	Delay1ms(5);

    g_Cali37Data = (g_Cali37Data << 8) | I2C_Random_R(I2C_Add_Cali37TP+1);
    g_CheckSum += g_Cali37Data;
	Delay1ms(5);

    g_Cali41Data = I2C_Random_R(I2C_Add_Cali41TP);
    g_CheckSum += g_Cali41Data;
	Delay1ms(5);

    g_Cali41Data = (g_Cali41Data << 8) | I2C_Random_R(I2C_Add_Cali41TP+1);
    g_CheckSum += g_Cali41Data;
	Delay1ms(5);

#if ET_FT ==1
#if Distence_En
    g_Distance = I2C_Random_R(Distance_5cm);
    g_CheckSum += g_Distance;
	Delay1ms(5);
    g_Distance = (g_Distance << 8) | I2C_Random_R(Distance_5cm+1);
    g_CheckSum += g_Distance;
	Delay1ms(5);	
#endif
#endif

    //读校验和并判断校验和，决定是否报错
	uErrFlag.g_ErrFlag = 0;
    if( I2C_Random_R(I2C_Add_CheckSum) != g_CheckSum )
	{
		uErrFlag.bits.Er5 = 1;
	}
	Delay1ms(5);

	//仅带语音款适用
	#if Have_Voice_Func
		g_LLCode = I2C_Random_R(I2C_Add_Voice);
		Delay1ms(5);
	#endif

    //判断黑体修正表格是表1还是表2
	uStaFlag.bits.TableNum = 0;
    if((I2C_Random_R(I2C_Add_Table) & 0x01) == 0x01)
   	    uStaFlag.bits.TableNum = 1;		//E2值为0，表1；	E2值为1，表2；
	Delay1ms(5);

	//耳温适用
#if ET_FT !=1
    g_Emission = I2C_Random_R(I2C_Add_Emission);		//人体发射率
	Delay1ms(5);

	//耳温适用
    g_HumanRatio = I2C_Random_R(I2C_Add_HumanRatio);	//人体系数
	Delay1ms(5);

	//耳套功能适用
    g_PcRatio = I2C_Random_R(I2C_Add_PcRatio);		//耳套系数
	Delay1ms(5);

	//仅适用于A款程序
    uStaFlag.bits.ProbeCover = 0;
    if((I2C_Random_R(I2C_Add_PcStatus) & 0x01) == 0x01)
   	    uStaFlag.bits.ProbeCover = 1; 		//耳套状态，0无耳套，1有耳套
	Delay1ms(5);
#endif
	uStaFlag.bits.Identify = 0;
	if( I2C_Random_R(I2C_Add_IdentifyCode) != IdentifyCode )
		uStaFlag.bits.Identify = 1;
	Delay1ms(5);

	I2C_Disable();
}

void Parm_AutoCheck(void)
{
	//EEPROM参数校验(如果校验和识别码都错则初始化数据，如果仅校验和错误则报错)
	Param_Check();
	if( uErrFlag.bits.Er5 )
	{
		if( uStaFlag.bits.Identify )
		{
			Param_Init();
			Param_Check();
			if( uErrFlag.bits.Er5 )
			{
				LED_CloseAll();
				Clr_Disp();
				Disp_ErrMsg();	//显示错误信息(此处设计为如果初始化成功，则不会显示错误信息，如果依然Er5则报错)
				while ( Port_Test )
				{			
					WDTR = 0x5A;	//喂狗
				}
				eSleepTask_Sta=Sleep_false;
				eMain_Task = Task_Sleepmode;
			}
		}
		else
		{
			Clr_Disp();
			LED_CloseAll();	//关闭LED
			Disp_ErrMsg();	//显示错误信息
			while ( Port_Test )
			{
				WDTR = 0x5A;	//喂狗
				if( !Port_Debug && !Port_Cal )
				{
					Param_Init();
					Disp_PAS();
				}
			}
			eMain_Task = Task_Sleepmode;
		}
	}
}

/**************************************************************************
函数名称：	void Param_Calistore(void)
函数功能：	默认参数设置
输入参数：	无
输出参数：	无
返回值  ：	无
占用空间：	TBD
备    注：	无
**************************************************************************/
void Param_Calistore(void)
{
	I2C_masterInit();
	g_CheckSum = 0x00;

	g_CheckSum += g_Cali25TP >> 8;
	I2C_Byte_W(I2C_Add_Cali25TP, g_Cali25TP >> 8);
	Delay1ms(5);

	g_CheckSum += g_Cali25TP ;
	I2C_Byte_W(I2C_Add_Cali25TP + 1, g_Cali25TP);
	Delay1ms(5);

	g_CheckSum += g_Cali37Data >> 8;
	I2C_Byte_W(I2C_Add_Cali37TP, g_Cali37Data >> 8);
	Delay1ms(5);

	g_CheckSum += g_Cali37Data;
	I2C_Byte_W(I2C_Add_Cali37TP + 1, g_Cali37Data);
	Delay1ms(5);

	g_CheckSum +=g_Cali41Data >> 8;
	I2C_Byte_W(I2C_Add_Cali41TP, g_Cali41Data >> 8);
	Delay1ms(5);

	g_CheckSum += g_Cali41Data;
	I2C_Byte_W(I2C_Add_Cali41TP + 1, g_Cali41Data);
	Delay1ms(5);

#if ET_FT ==1
#if Distence_En
    g_CheckSum +=g_Distance >> 8;
    I2C_Byte_W(Distance_5cm, g_Distance >> 8);
    Delay1ms(5);
    g_CheckSum += g_Distance;
    I2C_Byte_W(Distance_5cm + 1, g_Distance);
    Delay1ms(5);	
#endif
#endif 

	I2C_Byte_W(I2C_Add_CheckSum, g_CheckSum);
	Delay1ms(5);

	//检查校验和是否正确，如果错误显示Er5
	Param_Check();
	I2C_Disable();
}
