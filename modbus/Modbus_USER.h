#ifndef MODBUS_USER_H
#define MODBUS_USER_H
#include "APP.h"
/*********************************************************************
 * @author      Honokahqh
 *
 * @brief       Modbus slaver
 *
 * @Init:       ���ñ�����ַ                    MBS_SelfAddr
                ʹ��ʵ�ֵ�Modbus������          MBS_FUNCTION_xx_ENABLE
                ���ô����������                MBS_PhysicalSendBuff()
 *              ���üĴ���Map                   MBS_MappingInit()
                �����Ҫ������ֱ�Ӵ�����Hook��Ӧ�������뺯��
 * @Loop:       ���ս�������ú���MBS_CorePoll   
 *
 * @time:       2022.3.26
 */
/*Modbus slaver By Honokahqh*/
/***************************************************************/
#define MBS_SelfAddr    0x7D
#define Dev_Version     100

/***************************************************************/
#define	MBS_FUNCTION_01_ENABLE			        (1)			
#define	MBS_FUNCTION_02_ENABLE			        (0)
#define	MBS_FUNCTION_03_ENABLE			        (1)
#define	MBS_FUNCTION_04_ENABLE			        (0)
#define	MBS_FUNCTION_05_ENABLE			        (1)
#define	MBS_FUNCTION_06_ENABLE			        (1)
#define	MBS_FUNCTION_0F_ENABLE			        (1)
#define	MBS_FUNCTION_10_ENABLE			        (1)

#define USER_COIL_NUM					        (31)		
#define USER_HOLDREG_NUM				        (12)		
#define USER_DISINPUT_NUM				        (0)
#define USER_INPUTREG_NUM				        (0)

#define MBS_PORT_RXBUFF_SIZE			        64
#define MBS_PORT_TXBUFF_SIZE			        64

#define MBS_REG_ADC                             0 
#define MBS_Addr                                10
#define MBS_Ver                                 11

void MBS_MappingInit(void);
void MBS_Data_Init(void);
uint8 MBS_MemReadCoilState(uint16 coilAddr);
uint8 MBS_MemWriteCoilState(uint16 coilAddr, uint16 data);
uint16 MBS_MemWriteCoilsState(uint16 CoilAddr, uint16 Len, uint8 *data);
uint8 MBS_MemReadHoldRegValue(uint16 regAddr, uint8 *Value, uint8 num);
uint8 MBS_MemWriteHoldRegValue(uint16 regAddr, uint8 *Value, uint8 num);
#endif /* MODBUS_USER_H */
