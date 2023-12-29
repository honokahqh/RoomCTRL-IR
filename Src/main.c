#include "main.h"

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);

/**
 * main
 * @brief
 * @author Honokahqh
 * @date 2023-12-16
 */
int main(void)
{
    /* 配置系统时钟 */
    SystemClock_Config();
    IWDG_Config();
    uart2_init();
    timer1_init();
    ADC_init();
    infrared_init();
    MBS_MappingInit();
	LL_mDelay(500); // 等待RS485稳定
	uint8_t startframe[3] = {0xFF, MBS_SelfAddr, 0xFE};
    uart_send_data(startframe, 3);
    buzz_mode_set(3);
    System_Run();
}

/**
 * SystemClock_Config
 * @brief  system clock configration
 * @author Honokahqh
 * @date 2023-12-16
 */
static void SystemClock_Config(void)
{
    /* 使能HSI */
    LL_RCC_HSI_Enable();
    LL_RCC_HSI_SetCalibFreq(LL_RCC_HSICALIBRATION_24MHz);
    while (LL_RCC_HSI_IsReady() != 1)
    {
    }

    /* 设置 AHB 分频*/
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

    /* 配置HSISYS作为系统时钟源 */
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSISYS);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSISYS)
    {
    }

    /* 设置 APB1 分频*/
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_Init1msTick(24000000);

    /* 更新系统时钟全局变量SystemCoreClock(也可以通过调用SystemCoreClockUpdate函数更新) */
    LL_SetSystemCoreClock(24000000);

    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk;
}

/**
 * millis
 * @brief  获取系统运行时间，单位ms
 * @author Honokahqh
 * @date 2023-12-16
 */
unsigned int millis(void)
{
    return sys_ms;
}

/**
 * tick_printf
 * @brief 打印时间戳或tick值，用于调试程序
 * @author Honokahqh
 * @date 2023-12-16
 */
void tick_printf(const char *fmt, ...)
{
#if DEBUG
    RS485_TX_EN();
    va_list args;
    va_start(args, fmt);

    // 获取当前的tick值或时间戳
    unsigned long tick = millis();

    // 打印时间戳或tick值
    printf("[%lu] ", tick);

    // 打印剩余的信息
    vprintf(fmt, args);
    va_end(args);

    RS485_RX_EN();
#endif
}

/**
 * Error_Handler
 * @brief  This function is executed in case of error occurrence.
 * @author Honokahqh
 * @date 2023-12-16
 */
void Error_Handler(void)
{
    while (1)
    {
    }
}

#ifdef USE_FULL_ASSERT
/*******************************************************************************
**功能描述 ：输出产生断言错误的源文件名及行号
**输入参数 ：file：源文件名指针
**输入参数 ：line：发生断言错误的行号
**输出参数 ：
*******************************************************************************/
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT Xinling *****END OF FILE****/
