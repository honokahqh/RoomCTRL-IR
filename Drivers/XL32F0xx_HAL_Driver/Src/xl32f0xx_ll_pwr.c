/**
  ******************************************************************************
  * @file    xl32f0xx_ll_pwr.c
  * @author  MCU Application Team
  * @brief   PWR LL module driver.
  ******************************************************************************
  */
#if defined(USE_FULL_LL_DRIVER)

/* Includes ------------------------------------------------------------------*/
#include "xl32f0xx_ll_pwr.h"
#include "xl32f0xx_ll_bus.h"

/** @addtogroup XL32F0xx_LL_Driver
  * @{
  */

#if defined(PWR)

/** @defgroup PWR_LL PWR
  * @{
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
/** @addtogroup PWR_LL_Exported_Functions
  * @{
  */

/** @addtogroup PWR_LL_EF_Init
  * @{
  */

/**
  * @brief  De-initialize the PWR registers to their default reset values.
  * @retval An ErrorStatus enumeration value:
  *          - SUCCESS: PWR registers are de-initialized
  *          - ERROR: not applicable
  */
ErrorStatus LL_PWR_DeInit(void)
{
  /* Force reset of PWR clock */
  LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_PWR);

  /* Release reset of PWR clock */
  LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_PWR);

  return SUCCESS;
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
#endif /* defined(PWR) */
/**
  * @}
  */

#endif /* USE_FULL_LL_DRIVER */

/************************ (C) COPYRIGHT Xinling *****END OF FILE****/
