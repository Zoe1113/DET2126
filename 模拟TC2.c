#include <stdio.h>
#include <stdint.h>

// 存储输出寄存器值
unsigned char TC2CL, TC2RL;
unsigned char TC2CH, TC2RH;
unsigned char TC2DL, TC2DH;
unsigned long g_pwm_us;
int g_duty;

/**
 * @brief 计算TC2全套寄存器数值
 * @param PulseWidth PWM周期 单位us
 * @param DutyCycle 占空比 0~100
 */
void CalcTC2Param(unsigned long PulseWidth, unsigned char DutyCycle)
{
    unsigned long TC2R, TC2D;
    const unsigned long TC2rate = 250000UL;
    const unsigned long US_DIV = 1000000UL;
    unsigned long total_product;
    unsigned long int_part, frac_part;
    unsigned long total_cnt, duty_cnt;

    // 1. 计算 PulseWidth * TC2rate，拆分整数、余数（小数）
    total_product = PulseWidth * TC2rate;
    int_part  = total_product / US_DIV;
    frac_part = total_product % US_DIV;

    // 总周期计数四舍五入，和原浮点一致
    if(frac_part >= (US_DIV / 2))
        total_cnt = int_part + 1;
    else
        total_cnt = int_part;

    TC2R = 65536U - total_cnt;

    // 拆分TCR高低字节
    TC2RL = TC2R & 0xFF;
    TC2RH = (TC2R >> 8) & 0xFF;
    TC2CL = TC2RL;
    TC2CH = TC2RH;

    // 公式：duty_cnt = (int_part * DutyCycle * US_DIV + frac_part * DutyCycle + US_DIV/2 * 100) / (US_DIV * 100)
    unsigned long temp1 = int_part * DutyCycle;
    temp1 = temp1 * US_DIV;

    unsigned long temp2 = frac_part * DutyCycle;
    unsigned long numerator = temp1 + temp2 + 50000000UL; // 四舍五入补偿

    unsigned long denominator = US_DIV * 100UL;
    duty_cnt = numerator / denominator;

    TC2D = TC2R + duty_cnt;

    // 拆分TC2D高低字节
    TC2DL = TC2D & 0xFF;
    TC2DH = (TC2D >> 8) & 0xFF;
}

// 打印所有寄存器，带注释
void PrintRegValue(void)
{
    printf("========== TC2 寄存器计算结果 ==========\n");
    printf("    TC2CL = 0x%02X ;\t\t//%luus\n", TC2CL, g_pwm_us);
    printf("    TC2RL = 0x%02X ;\t\t//%luus\n", TC2RL, g_pwm_us);
    printf("    TC2CH = 0x%02X ;\t\t//%luus\n", TC2CH, g_pwm_us);
    printf("    TC2RH = 0x%02X ;\t\t//%luus\n", TC2RH, g_pwm_us);
    printf("    TC2DL6 = 0x%02X ;\t\t//%d%%\n", TC2DL, g_duty);
    printf("    TC2DH6 = 0x%02X ;\t\t//%d%%\n", TC2DH, g_duty);
    printf("========================================\n\n");
}

int main(void)
{
    unsigned long pwm_us;
    int duty;

    while (1)
    {
        printf("PWM周期(单位us)：");
        scanf("%lu", &pwm_us);

        printf("占空比(0~100)：");
        scanf("%d", &duty);

        if (duty < 0 || duty > 100)
        {
            printf("错误：占空比范围只能是 0 ~ 100！\n\n");
            continue;
        }

        // 保存全局参数用于打印注释
        g_pwm_us = pwm_us;
        g_duty = duty;
        CalcTC2Param(pwm_us, (unsigned char)duty);
        PrintRegValue();
    }

    return 0;
}
