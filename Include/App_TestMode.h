#ifndef _App_TestMode_H
#define _App_TestMode_H

//测量模式
typedef enum
{
	Earmode = 0,	//耳温模式
	Foreheadmode,	//额温模式
	Objectmode,		//物温模式
	Blackbodymode,	//黑体模式
	Insptectmode	//生产检验模式
}eTestmode;

//测试模式状态
typedef enum eTestmode_Status
{
	Test_Init = 0,
	Test_Get_Tp ,
	Test_Get_Ntc ,
	Test_Calculate ,
	Test_Disp ,
	Test_End
}eTestTaskSta;

extern eTestTaskSta eTestTask_Sta;
extern eTestmode eTestmode_num;
extern bit F_Disp_Dash;

void App_TestingMode(void);

#endif