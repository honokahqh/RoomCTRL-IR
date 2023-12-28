#include "app.h"

static struct pt Infrared_pt;
static struct pt uart_pt;
static struct pt pt100ms;

static int Task1_infrared_process(struct pt *pt);
static int Task2_uart_process(struct pt *pt);
static int Task3_100ms_process(struct pt *pt);

static void infrared_match(void);
static void ymodem_data_process(void);
static void ymodem_timeout_process(void);

static uint8_t infrared_Cnt;

void System_Run()
{
    PT_INIT(&Infrared_pt);
    while (1)
    {
        Task1_infrared_process(&Infrared_pt);
        Task2_uart_process(&uart_pt);
        Task3_100ms_process(&pt100ms);
    }
}

/**
 * Task1_infrared_process
 * @brief 红外数据处理
 * @author Honokahqh
 * @date 2023-12-16
 */
static int Task1_infrared_process(struct pt *pt)
{
    PT_BEGIN(pt);
    while (1)
    {
        PT_WAIT_UNTIL(pt, infrared_state.state == IR_END);
        infrared_state.state = IR_IDLE;
        infrared_match();
    }
    PT_END(pt);
}

/**
 * Task2_uart_process
 * @brief 串口数据处理 OTA/MBS
 * @author Honokahqh
 * @date 2023-12-16
 */
static int Task2_uart_process(struct pt *pt)
{
    PT_BEGIN(pt);
    while (1)
    {
//        PT_WAIT_UNTIL(pt, uart_state.has_data);
        ymodem_data_process();
        if (uart_state.rx_len <= 64 && ymodem_session.state == YMODEM_STATE_IDLE)
        {
            MBS_Buf._rxLen = uart_state.rx_len;
            memcpy(MBS_Buf._rxBuff, uart_state.rx_buff, uart_state.rx_len);
            MBS_CorePoll();
        }
        uart_state.has_data = 0;
        uart_state.rx_len = 0;
		PT_TIMER_DELAY(pt, 80);
    }
    PT_END(pt);
}

/**
 * Task3_100ms_process
 * @brief 100ms周期处理,mbs通讯超时/喂狗/红外通讯超时
 * @author Honokahqh
 * @date 2023-12-16
 */
extern volatile uint8_t mbs_has_data;
extern volatile uint16_t temperature;
static int Task3_100ms_process(struct pt *pt)
{
    static uint32_t cnt_100ms = 0, mbs_err_cnt = 0;
    PT_BEGIN(pt);
    while (1)
    {
        if (mbs_has_data == 0) // 300s mbs没消息重启
        {
            mbs_err_cnt++;
            if (mbs_err_cnt > 3000)
            {
                mbs_err_cnt = 0;
                NVIC_SystemReset();
            }
        }
        else
            mbs_err_cnt = 0;
        if (infrared_Cnt) // 红外收到信息后 1秒内应当被处理
        {
            infrared_Cnt--;
            if (mbs_has_data)
            {
                infrared_Cnt = 0;
                buzz_mode_set(1);
            }
            else if (infrared_Cnt == 0)
            {
                buzz_mode_set(2);
				uint8_t startframe[3] = {0xFF, MBS_SelfAddr, 0xFE};
				uart_send_data(startframe, 3);
            }
        }
        if (cnt_100ms % 10 == 0)
        {
            ymodem_timeout_process();
            ADC_start();
            LL_IWDG_ReloadCounter(IWDG);
            tick_printf("%s, ADC:%d\r\n", __func__, temperature);
        }
        mbs_has_data = 0;
        cnt_100ms++;
        LL_mDelay(120);
        PT_TIMER_DELAY(pt, 100);
    }
    PT_END(pt);
}

/**
 * infrared_match
 * @brief 红外码匹配
 * @author Honokahqh
 * @date 2023-12-16
 */
void infrared_match()
{
    int i;
    static uint32_t data[10] = {0xff00c738, 0xff005da2, 0xff009d62, 0xff001de2, 0xff00dd22, 0xff00fd02, 0xff003dc2, 0xff001fe0, 0xff0057a8, 0xff006f90};
    tick_printf("%s, IR code:%x  \r\n", __func__, infrared_state.data);
    for (i = 0; i < 10; i++)
    {
        if (infrared_state.data == data[i])
            break;
    }
    tick_printf("match:%d\r\n", i);
    mbsCoilValue[0].pData = 1;
    infrared_Cnt = 10; // 1秒倒计时
    mbs_has_data = 0;
    switch (i)
    {
    case 0: // 总开关
        mbsCoilValue[29].pData = 1;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        mbsCoilValue[i + 8].pData = 1;
        break;
    case 6:
        mbsCoilValue[24].pData = 1;
        break;
    case 7:
        mbsCoilValue[25].pData = 1;
        break;
    case 8:
        mbsCoilValue[26].pData = 1;
        break;
    case 9:
        for (uint8_t i = 0; i < 8; i++)
        {
            mbsCoilValue[i + 1].pData = 1;
        }
        break;
    default:
        infrared_Cnt = 0;
        buzz_mode_set(3);
        break;
    }
}

/**
 * ymodem_data_process
 * @brief ota协议 数据分析处理
 * @author Honokahqh
 * @date 2023-12-16
 */
void ymodem_data_process()
{
    static uint8_t IAP_Key[] = {0xff, MBS_Addr, 0x50, 0xA5, 0x5A, 0x38, 0x26, 0xFE};
    if (ymodem_session.state == YMODEM_STATE_IDLE)
    {
        for (uint8_t i = 0; i < 8; i++)
        {
            if (uart_state.rx_buff[i] != IAP_Key[i])
                return;
        }
        return;
    }
#if !Is_APP
    if (ymodem_session.state != YMODEM_STATE_IDLE)
    {
        uint8_t data[2];
        uint16_t len;
        int res = ymodem_packet_analysis(uart_state.rx_buff, uart_state.rx_len, data, &len);
        LL_mDelay(5);
        if (len > 0)
            uart_send_data(data, len);
        // if (res == 2) // 更新完成
        // {
        //     boot_to_app(APP_ADDR);
        // }
    }
#endif
}

/**
 * ymodem_timeout_process
 * @brief IAP通讯超时处理
 * @author Honokahqh
 * @date 2023-10-07
 */
static void ymodem_timeout_process()
{
#if !Is_APP
    if (ymodem_session.state != YMODEM_STATE_IDLE)
    { // 非lora模式下,OTA超时检测在此处而非Slaver_Period_1s
        uint8_t temp[2];
        ymodem_session.timeout++;
        if (ymodem_session.timeout > 3)
        {
            ymodem_session.error_count++;
            temp[0] = NAK;
            uart_send_data(temp, 1);
        }
        if (ymodem_session.error_count > 5)
        {
            memset(&ymodem_session, 0, sizeof(ymodem_session_t));
            temp[0] = CAN;
            temp[1] = CAN;
            uart_send_data(temp, 2);
            NVIC_SystemReset();
        }
    }
#endif
}
