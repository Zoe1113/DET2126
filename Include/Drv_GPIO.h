#ifndef _Drv_GPIO_H
#define _Drv_GPIO_H

#define Port_Mem    FP00    //记忆键
#define Port_Test   FP01    //开关/测量键
#define Port_Set   FP04    //设置键
#define Port_Mode    FP22    //耳套模式键
#define Port_Change_CF 	FP51	//高电平：可切换，低电平：不可切换
#define Port_CF 	FP52	//高电平：C，低电平：F
#define Port_Cal 	FP53	//校准模式检测入口
#define Port_Debug 	FP54	//绑定检测模式入口
#define Port_BZ     FP11    //蜂鸣口
#define Port_I2C_Power  FP15    //I2C供电口
#define Port_MOTOR     FP50    //马达


void Cal_Inspect_Detect(void);
void CF_Check(void);
void GPIO_Init(void);
void GPIO_PowerDown( void );

#endif
