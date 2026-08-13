#ifndef _Drv_LED_H
#define _Drv_LED_H


#define LED_Green_En() { FP55 = 0; }    //绿灯使能
#define LED_Green_Dis() { FP55 = 1; }   //绿灯关闭

#define LED_Red_En() { FP56 = 0; }      //红灯使能
#define LED_Red_Dis() { FP56 = 1; }     //红灯关闭

#define LED_Yellow_En() { FP57 = 0; }   //黄灯使能
#define LED_Yellow_Dis() { FP57 = 1; }  //黄灯关闭

void Drv_Led_Set(void);
void LED_CloseAll(void);


#endif