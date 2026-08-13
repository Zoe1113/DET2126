#include "Include.h"

void Drv_MT_Enable(void)
{
	Port_MOTOR=1;
}

void Drv_MT_Disable(void)
{
	Port_MOTOR=0;
}


void MT_Vib50(void)
{

	Drv_MT_Enable();
	Delay10ms(14);		//ясЁы140ms
	Drv_MT_Disable();
	Delay10ms(5);		//ясЁы50ms
}

void MT_Vib125(void)
{
	Drv_MT_Enable();
	Delay10ms(14);		//ясЁы140ms
	Drv_MT_Disable();
	Delay10ms(14);		//ясЁы140ms
}

void MT_Vib400(void)
{
	Drv_MT_Enable();
	Delay10ms(42);		//ясЁы420ms
	Drv_MT_Disable();
	Delay10ms(14);		//ясЁы140ms
}

void MT_Vib160(void)
{
	Drv_MT_Enable();
	Delay10ms(16);		//ясЁы160ms
	Drv_MT_Disable();
	Delay10ms(5);		//ясЁы140ms
}

void MT_Vib230(void)
{
	Drv_MT_Enable();
	Delay10ms(23);		//ясЁы230ms
	Drv_MT_Disable();
	Delay10ms(14);		//ясЁы140ms
}
