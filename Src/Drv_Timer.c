/**************************************************************************
文件名称：	Drv_Timer.c
说    明：	定时器设置（驱动层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
修订记录：
**************************************************************************/
#include "Include.h"


/**************************************************************************
	Timer 0 initialize(RTC，需使能外置32768 crystal)
**************************************************************************/


/**************************************************************************
	TC1 initialize(10ms)
	TC1R initial value = 65536 - (TC1 interrupt interval time * TC1 clock rate)
	TC1D initial value = TC1R + (PWM high pulse width period / TC1 clock rate)
**************************************************************************/
void TC1Init( void )
{
	TC1M = 0x40 ;		//0b01000000，Disable,Fcpu/8
	TC1CL = 0x3C ;
	TC1RL = 0x3C ;		//10ms（Fcpu = 2M)
	TC1CH = 0xF6 ;
	TC1RH = 0xF6 ;		//10ms（Fcpu = 2M)
	FTC1IRQ = 0;		//清TC1中断请求位
	FTC1IEN = 1;		//enable TC1中断
	FTC1ENB = 1 ;		//使能TC1定时器
}

/**************************************************************************
	TC2 initialize(Buzzer 4KHZ，PT10口)
	TC2R initial value = 65536 - (TC2 interrupt interval time * TC2 clock rate)
	TC2D initial value = TC2R + 占空比*(65536-TCR2)
**************************************************************************/
void TC2Init( void )
{
	TC2M = 0x40 ;		//0b01000000，Disable,Fcpu/8
    TC2CL = 0xC1 ;              //250us
    TC2RL = 0xC1 ;              //250us
    TC2CH = 0xFF ;              //250us
    TC2RH = 0xFF ;              //250us
    TC2DL6 = 0xE0 ;             //50%
    TC2DH6 = 0xFF ;             //50%
}

    unsigned long TC2R, TC2D;
    unsigned long total_cnt, duty_cnt;

void BuzzerTCInit(unsigned long PulseWidth, unsigned char DutyCycle)
{

	TC2M = 0x40 ;		//0b01000000，Disable,Fcpu/8

    // 边界保护
    if(DutyCycle > 100) DutyCycle = 100;
    if(PulseWidth == 0) PulseWidth = 1;

    // 约分简化公式，彻底消除超大乘法
    // total_cnt = PulseWidth * 250000 / 1000000 = PulseWidth / 4 四舍五入
    total_cnt = (PulseWidth + 2UL) / 4UL;

    TC2R = 65536U - total_cnt;

    // 拆分TC2R高低字节

    TC2RL = TC2R & 0xFF;
    TC2RH = (TC2R >> 8) & 0xFF;
    TC2CL = TC2RL;
    TC2CH = TC2RH;

    // 占空比约分公式：(PulseWidth * DutyCycle)/400 四舍五入
    duty_cnt = (PulseWidth * DutyCycle + 200UL) / 400UL;
    TC2D = TC2R + duty_cnt;

    // 拆分TC2D高低字节
    TC2DL6 = TC2D & 0xFF;
    TC2DH6 = (TC2D >> 8) & 0xFF;
}
