#ifndef __FLASH_DRIVER_H
#define __FLASH_DRIVER_H

#include "main.h"

#define BOOT_ADDR 0X08000000
#define APP_ADDR 0X08008000
#define Flash_End_ADDR 0x08010000
void JumpToApp(void);


#endif //
