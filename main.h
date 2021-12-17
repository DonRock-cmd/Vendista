#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gsm.h"
#include "LogMessages.h"
#include "GsmTask.h"
#include "Uart.h"
#include "MainTask.h"
#include "Ven.h"


#define _u8  unsigned char
#define _u16 unsigned short
/* USER CODE BEGIN EFP */


uint16_t bGetTimeInMs();
_u16 GetCRC16(const _u8 *buf, _u16 len);
void Error_Handler(void);


#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
