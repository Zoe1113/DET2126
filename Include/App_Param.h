#ifndef _App_Param_H
#define _App_Param_H

//EEPROM地址分配
#define I2C_Add_Cali25TP 		0x00	 	//I2C 25度校准值存储地址
#define I2C_Add_Cali37TP 		0x02	 	//I2C 37度校准值存储地址
#define I2C_Add_Cali41TP 		0x04	 	//I2C 41度校准值存储地址
#define I2C_Add_Table 			0x10	 	//I2C黑体表格选择存储地址
#define I2C_Add_Voice 			0x11	 	//I2C语音语言选择存储地址
#define I2C_Add_Unit 			0x12	 	//I2C单位选择存储地址
#if ET_FT !=1
    #define I2C_Add_Emission 		0x14	 	//I2C人体发射率存储地址
    #define I2C_Add_HumanRatio 		0x15		//I2C人体系数存储地址
    #define I2C_Add_PcRatio 		0x16	  	//I2C耳套系数存储地址
    #define I2C_Add_PcStatus 		0x17	 	//I2C耳套状态存储地址
#endif

#define I2C_Add_EarMem 			0x18		//I2C耳温模式记忆总记录数存储地址
#define I2C_Add_ForeMem 		0x19		//I2C额温模式记忆总记录数存储地址
#define I2C_Add_ObjMem 			0x1A		//I2C物温模式记忆总记录数存储地址
#define I2C_Add_CheckSum 		0x1E	 	//I2C校准值的校验和存储地址(校验和一定识别码之前写入，地址不一定要在前面)
#define I2C_Add_IdentifyCode 	0x1F 		//I2C识别码存储地址

#define I2C_Add_Offset 			0x03 		//I2C耳温、额温、物温模式当前记录号相对总记录号地址偏移量

#define Mem_EarAdd 				0x20		//I2C耳温记忆截止地址
#define Mem_ForeAdd 			0x66		//I2C额温记忆截止地址
#define Mem_ObjAdd 				0xAC		//I2C物温记忆截止地址

#define IdentifyCode            0x37        //识别码
/****************************************系数相关RAM定义****************************************/
#if ET_FT !=1
    extern uint8 g_HumanRatio;			//人体系数
    extern uint8 g_Emission;			//发射率
    extern uint8 g_PcRatio;				//耳套修正
#endif

extern uint8 g_CheckSum;			//CRC校验位


void Param_Init(void);
void Param_Check(void);
void Parm_AutoCheck(void);
void Param_Calistore(void);

#endif