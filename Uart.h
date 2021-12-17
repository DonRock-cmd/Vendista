#ifndef INC_UART_H_
#define INC_UART_H_

#include "main.h"
#include "LogMessages.h"
#include "gsm.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#define GsmRxBuffSize			300
#define VenRxBuffSize			500
#define VenTxBuffSize			300



void vGsmStartRx(void);
void vVenStartRx(void);

// Buffers and variables for Gsm (UART variables)
uint8_t GsmRxBuff[GsmRxBuffSize];
extern uint16_t GsmRxBuffCounter;
uint8_t GsmRxTemporaryCounter;
uint8_t GsmInByte;

// Buffers and variables for Vendista (UART variables)
uint8_t VenRxBuff[VenRxBuffSize];
extern uint16_t VenRxBuffCounter;
uint8_t VenInByte;
uint8_t VenTxBuff[VenTxBuffSize];

#endif /* INC_UART_H_ */
