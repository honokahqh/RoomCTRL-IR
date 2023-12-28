#ifndef __BSP_DRIVER_H
#define __BSP_DRIVER_H

#include "main.h"
#include "xl32f003xx_ll_Start_Kit.h"
#include "stdint.h"
/* 串口 */
#define UART_TX_PORT GPIOA
#define UART_TX_PIN LL_GPIO_PIN_2
#define UART_RX_PORT GPIOA
#define UART_RX_PIN LL_GPIO_PIN_1
#define RS485_RT_PORT GPIOA
#define RS485_RT_PIN LL_GPIO_PIN_5

#define RS485_TX_EN() LL_GPIO_SetOutputPin(RS485_RT_PORT, RS485_RT_PIN)
#define RS485_RX_EN() LL_GPIO_ResetOutputPin(RS485_RT_PORT, RS485_RT_PIN)

#define UART_IDLE_Timeout 50 // 50 * 100us = 5ms
typedef struct
{
	uint8_t busy;	  // 串口忙
	uint8_t IDLE;	  // 串口空闲-0:空闲
	uint8_t has_data; // 串口一帧数据接收完成

	uint8_t rx_buff[256];
	uint8_t rx_len;
} uart_state_t;
extern uart_state_t uart_state;

void uart2_init(void);
void uart_send_data(uint8_t *data, uint8_t len);

// timer1 1ms定时
extern volatile uint32_t sys_ms;
void timer1_init(void);
#define BUZZ_PORT GPIOA
#define BUZZ_PIN LL_GPIO_PIN_3
void buzz_mode_set(uint8_t mode);
// adc
void ADC_init(void);
void ADC_start(void);

extern volatile uint16_t uhADCxConvertedData_Voltage_mVolt;

// 红外
typedef enum
{
    IR_IDLE = 0,
    IR_START,
    IR_PROCESS,
    IR_END,
} IR_frame_state_t;

typedef struct
{
    /* data */
    uint32_t data;
    uint8_t bit_count;
    IR_frame_state_t state;
	
	uint32_t temp_buf[64];
	uint8_t temp_len;
} IR_state_t;
extern volatile IR_state_t infrared_state;

void infrared_init(void);

// 看门狗
void IWDG_Config(void);
#endif // 
