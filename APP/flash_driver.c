// #include <stdint.h>
#include "flash_driver.h"

typedef void (*AppFunction)(void);

void JumpToApp()
{
    // 关闭所有中断
    __disable_irq();

    // 重新定位中断向量表到APP的起始地址
    SCB->VTOR = APP_ADDR;

    // 读取APP的堆栈指针（APP的第一个字）
    uint32_t appStack = *(volatile uint32_t *)APP_ADDR;

    // 读取APP的复位中断服务程序地址（APP的第二个字）
    uint32_t appEntry = *(volatile uint32_t *)(APP_ADDR + 4);

    // 设置堆栈指针
    __set_MSP(appStack);

    // 跳转到APP的入口点
    AppFunction app = (AppFunction)appEntry;
    app();
}

#define __RAM_FUNC __attribute__((section(".RamFunc")))

static __RAM_FUNC void FLASH_Program_Page(uint32_t Address, uint32_t *DataAddress)
{

    uint8_t index = 0;
    uint32_t dest = Address;
    uint32_t *src = DataAddress;
    uint32_t primask_bit;

    SET_BIT(FLASH->CR, FLASH_CR_PG);
    /* Enter critical section: row programming should not be longer than 7 ms */
    primask_bit = __get_PRIMASK();
    __disable_irq();
    /* 32 words*/
    while (index < 32U)
    {
        *(uint32_t *)dest = *src;
        src += 1U;
        dest += 4U;
        index++;
        if (index == 31)
        {
            SET_BIT(FLASH->CR, FLASH_CR_PGSTRT);
        }
    }

    /* Exit critical section: restore previous priority mask */
    __set_PRIMASK(primask_bit);
}

uint32_t flash_write_buf[32];
void flash_program_bytes(uint32_t write_addr, uint8_t *data, uint32_t len)
{
    uint8_t *index;
    index = data;
    if (write_addr < APP_ADDR && write_addr + len > Flash_End_ADDR)
        return;
    while (len)
    {
        memset(flash_write_buf, 0, 128);
        if (len >= 128)
        {
            memcpy(flash_write_buf, index, 128);
            FLASH_Program_Page(write_addr, (uint32_t *)flash_write_buf);
            len -= 128;
            index += 128;
        }
        else
        {
            memcpy(flash_write_buf, index, len);
            FLASH_Program_Page(write_addr, (uint32_t *)flash_write_buf);
            len = 0;
        }
    }
}