/**************************************************************************
文件名称：	APP_Isr.c
说    明：	中断服务程序（应用层）
作    者:	Liaoy
版    本：	V1.0
芯	  片:	SN8P29XX
修订记录：
**************************************************************************/
#include "Include.h"

/**************************************************************************
	Interrupt service program
**************************************************************************/
void __interrupt[0x08] Interrupt_pro(void)
{
	if(FT0IRQ)
	{
		FT0IRQ=0;
		F_500ms =1;		//set 500ms flag
	}
	else if(FADC1IRQ)
	{
		FADC1IRQ = 0;
	}
	else if(FTC0IRQ)
	{
		FTC0IRQ =0;
	}
	else if(FTC1IRQ)
	{
		FTC1IRQ = 0;
		F_10ms = 1 ;	//set 10ms flag
		F_10ms_task = 1;		
	}
	else if(FP00IRQ)
	{
		FP00IRQ = 0;
	}
	else if(FP01IRQ)
	{
		FP01IRQ = 0;
	}
	else if(FUTX1IRQ)
	{
		FUTX1IRQ = 0;
	}
	else if(FURX1IRQ)
	{
		FURX1IRQ = 0;
	}
	else
	{
	}
}
