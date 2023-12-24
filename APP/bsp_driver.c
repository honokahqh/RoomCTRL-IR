#include "bsp_driver.h"
uart_state_t uart_state;

/**
 * uart2_init
 * @brief uart2初始化
 * @author Honokahqh
 * @date 2023-12-16
 */
void uart2_init()
{
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);

    LL_GPIO_SetPinMode(RS485_RT_PORT, RS485_RT_PIN, LL_GPIO_MODE_OUTPUT);

    LL_GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate = LL_GPIO_AF4_USART2;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_1;
    GPIO_InitStruct.Alternate = LL_GPIO_AF9_USART2;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    NVIC_SetPriority(USART2_IRQn, 0);
    NVIC_EnableIRQ(USART2_IRQn);

    LL_USART_InitTypeDef USART_InitStruct;
    USART_InitStruct.BaudRate = 9600;
    USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
    USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
    LL_USART_Init(USART2, &USART_InitStruct);
    LL_USART_ConfigAsyncMode(USART2);

    //    LL_USART_EnableIT_PE(USART2);
    //    LL_USART_EnableIT_ERROR(USART2);
    LL_USART_EnableIT_RXNE(USART2);

    LL_USART_Enable(USART2);
}

/**
 * uart_send_data
 * @brief uart 485发送数据
 * @author Honokahqh
 * @date 2023-12-16
 */
void uart_send_data(uint8_t *data, uint8_t len)
{
    RS485_TX_EN();
    for (uint8_t i = 0; i < len; i++)
    {
        USART2->DR = data[i];
        while (!(USART2->SR & 0x40))
            ;
    }
	LL_mDelay(3);// 可以不等待USART2->SR & 0x40是发送完成标志位,但是第一次发送时需要等待
    RS485_RX_EN();
}

/**
 * USART2_IRQHandler
 * @brief 串口接收中断处理
 * @author Honokahqh
 * @date 2023-12-16
 */
void USART2_IRQHandler()
{
    if (USART2->SR & USART_SR_RXNE)
    {
        uart_state.rx_buff[uart_state.rx_len++] = USART2->DR;
        uart_state.IDLE = 1;
    }
}

/**
 * timer1_init
 * @brief 1ms定时器初始化
 * @author Honokahqh
 * @date 2023-12-16
 */
uint32_t sys_ms, sys_100us;
void timer1_init()
{
    LL_APB1_GRP2_EnableClock(RCC_APBENR2_TIM1EN);

    /*配置TIM1*/
    LL_TIM_InitTypeDef TIM1CountInit = {0};

    /***********************************************
    ** 输入时钟：    8000000
    ** 计数模式：    向上计数
    ** 时钟预分频：  8000
    ** 自动重装载值：500
    ** 重复计数值：  0
    ************************************************/
    TIM1CountInit.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    TIM1CountInit.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIM1CountInit.Prescaler = 80 - 1;
    TIM1CountInit.Autoreload = 29 - 1;
    TIM1CountInit.RepetitionCounter = 0;

    /*初始化TIM1*/
    LL_TIM_Init(TIM1, &TIM1CountInit);

    /*使能UPDATE中断*/
    LL_TIM_EnableIT_UPDATE(TIM1);

    LL_GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = LL_GPIO_PIN_3;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*使能TIM1计数器*/
    LL_TIM_EnableCounter(TIM1);

    /*开启UPDATE中断请求*/
    NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
    NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 0);
}

/**
 * buzz_mode_set
 * @brief 设置蜂鸣器发生模式
 * @author Honokahqh
 * @date 2023-12-16
 */
uint16_t buzz_buffer[10], buzz_index; // 开关时间,开-关-开-关-开-关-开-关-开-关
void buzz_mode_set(uint8_t mode)
{
    switch (mode)
    {
    case 1:
        buzz_index = 0;
        memset(buzz_buffer, 0, sizeof(buzz_buffer));
        buzz_buffer[0] = 200;
        break;
    case 2:
        buzz_index = 0;
        memset(buzz_buffer, 0, sizeof(buzz_buffer));
        buzz_buffer[0] = 100;
        buzz_buffer[1] = 100;
        buzz_buffer[2] = 100;
        break;
	case 3:
		buzz_index = 0;
		memset(buzz_buffer, 0, sizeof(buzz_buffer));
        buzz_buffer[0] = 500;
    default:
        break;
    }
}

/**
 * TIM1_BRK_UP_TRG_COM_IRQHandler
 * @brief timer1 1ms溢出事件
 * @author Honokahqh
 * @date 2023-12-16
 */
void TIM1_BRK_UP_TRG_COM_IRQHandler(void)
{
    if ((LL_TIM_ReadReg(TIM1, SR) & LL_TIM_SR_UIF) == 1 && LL_TIM_IsEnabledIT_UPDATE(TIM1))
    {
        LL_TIM_ClearFlag_UPDATE(TIM1);
        if (sys_100us < 1000)
            sys_100us++;
        if (uart_state.IDLE)
        {
            uart_state.IDLE++;
            if (uart_state.IDLE > UART_IDLE_Timeout)
            {
                uart_state.IDLE = 0;
                uart_state.has_data = 1;
            }
        }
    }
}

/**
 * ADC_init
 * @brief ADC初始化,用于温度采集
 * @author Honokahqh
 * @date 2023-12-16
 */
/* Private define ------------------------------------------------------------*/
#define ADC_DELAY_CALIB_ENABLE_CPU_CYCLES (LL_ADC_DELAY_CALIB_ENABLE_ADC_CYCLES * 32)
#define VAR_CONVERTED_DATA_INIT_VALUE (__LL_ADC_DIGITAL_SCALE(LL_ADC_RESOLUTION_12B) + 1)

void ADC_init()
{
    LL_ADC_Reset(ADC1);
    /* 使能GPIOA时钟 */
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);

    /* 配置管脚PA4为模拟输入 */
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_0, LL_GPIO_MODE_ANALOG);

    /* 使能ADC1时钟 */
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_ADC1);

    if (__LL_ADC_IS_ENABLED_ALL_COMMON_INSTANCE() == 0)
    {
        /* 配置内部转换通道 */
        LL_ADC_SetCommonPathInternalCh(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_PATH_INTERNAL_NONE);
    }

    /* 设置ADC时钟 */
    LL_ADC_SetClock(ADC1, LL_ADC_CLOCK_SYNC_PCLK_DIV2);

    /* 设置12位分辨率 */
    LL_ADC_SetResolution(ADC1, LL_ADC_RESOLUTION_12B);

    /* 设置数据右对齐 */
    LL_ADC_SetResolution(ADC1, LL_ADC_DATA_ALIGN_RIGHT);

    /* 设置低功耗模式无 */
    LL_ADC_SetLowPowerMode(ADC1, LL_ADC_LP_MODE_NONE);

    /* 设置通道转换时间 */
    LL_ADC_SetSamplingTimeCommonChannels(ADC1, LL_ADC_SAMPLINGTIME_41CYCLES_5);

    /* 设置触发源为TIM1 TRGO */
    LL_ADC_REG_SetTriggerSource(ADC1, LL_ADC_REG_TRIG_SOFTWARE);

    /* 设置转换模式为单次转换 */
    LL_ADC_REG_SetContinuousMode(ADC1, LL_ADC_REG_CONV_SINGLE);

    /* 设置DMA模式为不开启 */
    LL_ADC_REG_SetDMATransfer(ADC1, LL_ADC_REG_DMA_TRANSFER_NONE);

    /* 设置过载管理模式为覆盖上一个值 */
    LL_ADC_REG_SetOverrun(ADC1, LL_ADC_REG_OVR_DATA_OVERWRITTEN);

    /* 设置不连续模式为不使能 */
    LL_ADC_REG_SetSequencerDiscont(ADC1, LL_ADC_REG_SEQ_DISCONT_DISABLE);

    /* 设置通道4为转换通道 */
    LL_ADC_REG_SetSequencerChannels(ADC1, LL_ADC_CHANNEL_0);

    /* 使能EOC中断 */
    LL_ADC_EnableIT_EOC(ADC1);

    NVIC_SetPriority(ADC_COMP_IRQn, 0);
    NVIC_EnableIRQ(ADC_COMP_IRQn);
}

/**
 * ADC_start
 * @brief 采样
 * @author Honokahqh
 * @date 2023-12-16
 */
void ADC_start(void)
{
    __IO uint32_t wait_loop_index = 0;
#if (USE_TIMEOUT == 1)
    uint32_t Timeout = 0;
#endif

    if (LL_ADC_IsEnabled(ADC1) == 0)
    {
        /* 使能校准 */
        LL_ADC_StartCalibration(ADC1);

#if (USE_TIMEOUT == 1)
        Timeout = ADC_CALIBRATION_TIMEOUT_MS;
#endif

        while (LL_ADC_IsCalibrationOnGoing(ADC1) != 0)
        {
#if (USE_TIMEOUT == 1)
            /* 检测校准是否超时 */
            if (LL_SYSTICK_IsActiveCounterFlag())
            {
                if (Timeout-- == 0)
                {
                }
            }
#endif
        }

        /* ADC校准结束和使能ADC之间的延时 */
        wait_loop_index = (ADC_DELAY_CALIB_ENABLE_CPU_CYCLES >> 1);
        while (wait_loop_index != 0)
        {
            wait_loop_index--;
        }

        /* 使能ADC */
        LL_ADC_Enable(ADC1);
    }
    LL_ADC_REG_StartConversion(ADC1);
}

/**
 * TAB_WENDU_10K
 * @brief 3950 10K温敏电阻配合4.7k分压电阻
 * @author Honokahqh
 * @date 2023-12-16
 */
const uint16_t TAB_WENDU_10K[] = {
    3924, 3915, 3906, 3896, 3886, 3876, 3865, 3853, 3842, 3829 //-30~ -21
    ,
    3817, 3804, 3790, 3776, 3761, 3746, 3730, 3714, 3698, 3682 //-20~ -11
    ,
    3663, 3644, 3626, 3606, 3586, 3566, 3545, 3523, 3501, 3478 //-10~  -1
    ,
    3455, 3431, 3407, 3382, 3356, 3330, 3304, 3277, 3249, 3221 //  0~  9
    ,
    3192, 3163, 3134, 3104, 3073, 3042, 3011, 2979, 2947, 2914 // 10~ 19
    ,
    2881, 2848, 2815, 2781, 2747, 2713, 2678, 2643, 2608, 2573 // 20~ 29
    ,
    2538, 2503, 2467, 2432, 2397, 2361, 2325, 2290, 2255, 2219 // 30~ 39
    ,
    2184, 2149, 2107, 2079, 2044, 2010, 1975, 1941, 1907, 1874 // 40~ 49
    ,
    1840, 1807, 1774, 1742, 1710, 1678, 1647, 1615, 1585, 1554 // 50~ 59
    ,
    1524, 1495, 1466, 1437, 1409, 1381, 1353, 1326, 1299, 1273 // 60~ 69
    ,
    1247}; // 70

volatile uint16_t temperature;
volatile uint16_t uhADCxConvertedData = VAR_CONVERTED_DATA_INIT_VALUE;
volatile uint16_t uhADCxConvertedData_Voltage_mVolt = 0;

void APP_AdcGrpRegularUnitaryConvCompleteCallback()
{
    uint8_t i;
    uint16_t adc_data;
    adc_data = LL_ADC_REG_ReadConversionData12(ADC1);
    for (i = 0; i < sizeof(TAB_WENDU_10K) / sizeof(TAB_WENDU_10K[0]); i++)
    {
        if (adc_data > TAB_WENDU_10K[i])
        {
            temperature = i - 30;
            break;
        }
    }
    // 计算小数点
    temperature = temperature * 10;
    if (i > 0)
        temperature += ((double)(adc_data - TAB_WENDU_10K[i]) / (double)(TAB_WENDU_10K[i] - TAB_WENDU_10K[i - 1])) * 10;
    mbsHoldRegValue[MBS_REG_ADC].pData = temperature;
}

void ADC_COMP_IRQHandler(void)
{
    /* 检测是不是转换结束触发的中断 */
    if (LL_ADC_IsActiveFlag_EOC(ADC1) != 0)
    {
        /* 清空ADC EOC 中断 */
        LL_ADC_ClearFlag_EOC(ADC1);

        /* 调用中断处理函数 */
        APP_AdcGrpRegularUnitaryConvCompleteCallback();
    }
}

volatile IR_state_t infrared_state;

void infrared_init()
{
    // gpioA pin4 上升沿中断
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_4, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_4, LL_GPIO_PULL_UP);
    LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
    EXTI_InitStruct.Line = LL_EXTI_LINE_4;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
    LL_EXTI_Init(&EXTI_InitStruct);
    NVIC_SetPriority(EXTI4_15_IRQn, 0);
    NVIC_EnableIRQ(EXTI4_15_IRQn);
}

void EXTI4_15_IRQHandler()
{
    // 清除中断
    LL_EXTI_ClearFlag(LL_EXTI_LINE_4);
    // if (sys_100us >= 1000)
    //     infrared_state.temp_len = 0;
    // infrared_state.temp_buf[infrared_state.temp_len++] = sys_100us;
    // sys_100us = 0;
    if (sys_100us >= 1000 && infrared_state.state == IR_IDLE)
    {
        infrared_state.state = IR_START;
        infrared_state.bit_count = 0;
        sys_100us = 0;
    }
    else if (infrared_state.state == IR_START)
    {
        if (sys_100us < 0x40 && sys_100us > 0x30)
        {
            infrared_state.state = IR_PROCESS;
            sys_100us = 0;
        }
        else
            infrared_state.state = IR_IDLE;
    }
    else if (infrared_state.state == IR_PROCESS)
    {
        infrared_state.data <<= 1;
        if (sys_100us < 15 && sys_100us > 8)
            infrared_state.data |= 1;
        else if (sys_100us < 26 && sys_100us > 19)
            infrared_state.data &= ~0;
        else
            infrared_state.state = IR_IDLE;
        infrared_state.bit_count++;
        sys_100us = 0;
        if (infrared_state.bit_count == 32)
            infrared_state.state = IR_END;
    }
}

void IWDG_Config(void)
{
    /* 使能LSI */
    LL_RCC_LSI_Enable();
    while (LL_RCC_LSI_IsReady() == 0U)
    {
        ;
    }

    /* 使能IWDG */
    LL_IWDG_Enable(IWDG);
    /* 开启写权限 */
    LL_IWDG_EnableWriteAccess(IWDG);
    /* 设置IWDG分频 */
    LL_IWDG_SetPrescaler(IWDG, LL_IWDG_PRESCALER_32); // T=1MS
    /* 设置喂狗事件*/
    LL_IWDG_SetReloadCounter(IWDG, 3000); // 1ms*1000=1s
    /* IWDG初始化*/
    while (LL_IWDG_IsReady(IWDG) == 0U)
    {
        ;
    }
    /*喂狗*/
    LL_IWDG_ReloadCounter(IWDG);
}
