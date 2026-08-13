#ifndef _Drv_LCD_ET05_H
#define _Drv_LCD_ET05_H

//底层驱动
sfr	lcd0 = 0xf00 ;
sfr	lcd1 = 0xf01 ;
sfr	lcd2 = 0xf02 ;
sfr	lcd3 = 0xf03 ;
sfr	lcd4 = 0xf04 ;
sfr	lcd5 = 0xf05 ;
sfr	lcd6 = 0xf06 ;
sfr	lcd7 = 0xf07 ;
sfr	lcd8 = 0xf08 ;
sfr	lcd9 = 0xf09 ;
sfr	lcd10 = 0xf0a ;
sfr	lcd11 = 0xf0b ;
sfr	lcd12 = 0xf0c ;
sfr	lcd13 = 0xf0d ;
sfr	lcd14 = 0xf0e ;
sfr	lcd15 = 0xf0f ;
sfr lcd16 = 0xf10 ;

//常用lcd符号宏定义
#define lcd_voice 0x02  //声音符号  S17 lcd8 @COM2
#define lcd_full_bat 0x02       //电池内部显示 S19 lcd7 @COM2
#define lcd_bat 0x01       //电池外部显示  S18 lcd7 @COM1
#define lcd_person0_3 0x04    //0-3月符号 W1 lcd8 @COM3
#define lcd_person3_36 0x01    //3-36月符号 W2 lcd8 @COM1
#define lcd_person 0x08    //月份轮廓符号 W4 lcd8 @COM4
#define lcd_person36_up 0x08    //36月+符号 W3 lcd7 @COM4
#define lce_one 0x01 //温度前的1 S1 lcd6 @COM1
#define lcd_point 0x01   //显示温度的小数点.符号 S5 lcd2 @COM1
#define lcd_unit_c 0x08 //温度单位符号C符号 S10 lcd0 @COM4
#define lcd_unit_f 0x04 //温度单位符号F符号 S9 lcd0 @COM3
#define lcd_smileface 0x01   //笑脸 S2 lcd9 @COM1
#define lcd_badface 0x02  //哭脸 S3 lcd9 @COM2
#define lcd_mem 0x04    //M符号 S4 lcd9 @COM3
#define lcd_ear 0x08    //耳温符号 S6 lcd9 @COM4
#define lcd_forehead 0x01   //额温符号 S7 lcd0 @COM1
#define lcd_obj 0x02    //物温符号 S8 lcd0 @COM2


//显示指定lcd图标
#define lcd_mem_en()    { lcd9 |= lcd_mem; }	    //M点亮
#define lcd_bat_en()    { lcd7 |= lcd_bat; }       //电池外部显示
#define lcd_fullbat_en()        { lcd7 |= lcd_full_bat; }  //电池内部显示
#define lcd_point_en()      { lcd2 |= lcd_point; }  //小数点
#define lcd_badface_en()    { lcd9 |= lcd_badface; }  //哭脸
#define lcd_smileface_en()  { lcd9 |= lcd_smileface; }  //笑脸
#define lcd_ear_en()        { lcd9 |= lcd_ear; }    //耳温符号
#define lcd_forehead_en()   { lcd0 |= lcd_forehead; }  //额温符号
#define lcd_obj_en()        { lcd0 |= lcd_obj; }    //物温符号
#define lcd_unit_c_en()     { lcd0 |= lcd_unit_c; }  //温度单位C
#define lcd_unit_f_en()     { lcd0 |= lcd_unit_f; }  //温度单位F
#define lcd_voice_en()   { lcd8 |= lcd_voice; }  //声音符号
#define lcd_one_en()   { lcd6 |= lce_one; }  //温度前的1
#define lcd_person_en()   { lcd8 |= lcd_person; }  //月份轮廓符号
#define lcd_person0_3_en() { lcd8 |= lcd_person0_3; }  //0-3月符号
#define lcd_person3_36_en() { lcd8 |= lcd_person3_36; }  //3-36月符号
#define lcd_person36_up_en() { lcd7 |= lcd_person36_up; }  //36月+符号



//清除指定lcd图标
#define lcd_mem_clr() 	    { lcd9 &= ~lcd_mem; }	//M清除
#define lcd_bat_clr()       { lcd7 &= ~lcd_bat; }   //电池外部显示清除
#define lcd_fullbat_clr()        { lcd7 &= ~lcd_full_bat; }  //电池内部显示清除
#define lcd_point_clr()     { lcd2 &= ~lcd_point; }  //小数点清除
#define lcd_badface_clr()   { lcd9 &= ~lcd_badface; }  //哭脸清除
#define lcd_smileface_clr() { lcd9 &= ~lcd_smileface; }  //笑脸清除
#define lcd_ear_clr()       { lcd9 &= ~lcd_ear; }    //耳温符号清除
#define lcd_forehead_clr()  { lcd0 &= ~lcd_forehead; }  //额温符号清除
#define lcd_obj_clr()       { lcd0 &= ~lcd_obj; }    //物温符号清除
#define lcd_unit_c_clr()     { lcd0 &= ~lcd_unit_c; }  //温度单位C清除
#define lcd_unit_f_clr()     { lcd0 &= ~lcd_unit_f; }  //温度单位F清除
#define lcd_voice_clr()   { lcd8 &= ~lcd_voice; }  //声音符号清除
#define lcd_one_clr()   { lcd6 &= ~lce_one; }  //温度前的1清除
#define lcd_person_clr()   { lcd8 &= ~lcd_person; }  //月份轮廓符号清除
#define lcd_person0_3_clr() { lcd8 &= ~lcd_person0_3; }  //0-3月符号清除
#define lcd_person3_36_clr() { lcd8 &= ~lcd_person3_36; }  //3-36月符号清除
#define lcd_person36_up_clr() { lcd7 &= ~lcd_person36_up; }  //36月+符号清除


//消隐指定lcd图标
#define lcd_bat_xor()        { lcd7 ^= lcd_bat; }

extern uint16 __ROM	DispTable[];

void Lcd_Init(void);
void Disp_Version(uint16 num);
void Disp_Code(uint16 num);
void Clr_Disp(void);
void Clr_Disp888(void);
void Clr_Disp_KeepBat(void);  // 清屏但保留电池图标
void Disp_All(void);
void Disp_FullBat(void);
void Disp_LowBat(void);
void Disp_Unit(void);
void Disp_BadFace(void);
void Disp_SmileFace(void);
void Disp_ModeSign(void);
void Clr_ModeSign(void);
void Disp_OFF(void);
void Disp_On(void);
void Disp_Lo(void);
void Disp_Hi(void);
void Disp_Ready(void);
void Disp_Null(void);
void Disp_ErN(uint8 num);
void Disp_ErrMsg(void);
void Disp_NtcEr2(void);
void Disp_CAL(void);
void Disp_Ab(void);
void Disp_PAS(void);
void Disp_Err(void);
void Disp_Debug1(void);
void Disp_Debug2(void);
void Disp_DebugPASn(uint8 num);
void Disp_12H(void);
void Disp_24H(void);
void Disp_Ch(void);
void Disp_En(void);
void Disp_SP(void);
void Disp_Table1(void);
void Disp_Table2(void);
void Disp_Temp(bit Point, bit High, bit Unit, int16 Temp);
void Disp_Ntc(uint16 Temp);
void Disp_Time( bit F_time, bit F_Format, uint8 left, uint8 right);
void Disp_Year(uint16 L_buf);
void Clr_SetTime(uint8 L_Blink);
void Disp_CAP(void);
void Disp_VoiceSign(void);
void LVD_Display(void);
void Disp_ForeheadLoading(void);
void Disp_FourSecLoop(void);
void Disp_FourSecLoop_Init(void);
uint8 Disp_FourSecLoop_Step(void);
void Disp_ForeheadLoading_Init(void);
bit Disp_ForeheadLoading_Process(void);
void Disp_ForeheadLoading_Stop(void);

extern uint8 g_ForeheadLoading_Sta;
extern uint8 g_ForeheadLoading_Timer;


#endif
/*************************************************************************/