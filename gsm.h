
#ifndef GSM_H_
#define GSM_H_


#include "main.h"
#include "string.h"
#include "cmsis_os.h"
#include "stdlib.h"
#include "stdio.h"
#include <LogMessages.h>
#include "Uart.h"
#include "Ven.h"
#include "MainTask.h"


#define TimeoutTx 	  			500
#define TimeoutPwr    			500
#define TimeoutDataIn  			500
#define TimeoutCheckConnection  50
#define TimeoutAnswer  			50
#define TimeoutCGATT 	  		1000
#define RegTimeout     			3000
#define TCPTimeout     			2000
#define Answer_OK 	  			1
#define Answer_Err 	  			2
#define Gsm_Err 	 			-1
#define Repeat_AT_command		10
#define GsmErrorSim				1

//
#define CloseConnect
//

extern uint8_t GsmRxBuff[];
extern uint16_t GsmRxBuffCounter;
extern uint8_t GsmRxTemporaryCounter;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart2;
typedef struct
{
	uint8_t connect; // 1: Connected to modem, 		  0: Not connected
	uint8_t sim;     // 1: SIM card detected, 		  0: Not detected
	uint8_t reg; 	 // 1: Registered on the network, 0: Not registered.
	uint8_t net; 	 // 1: Connected with server,     0: Not connected
	uint16_t errors;
}
xGsmStateTypeDef;
extern xGsmStateTypeDef xGsmState;
typedef struct
{
	uint16_t size;
	uint8_t *Buffer;
}xGsmToServerTypeDef;
extern xGsmToServerTypeDef xServerTxDataTo;

uint8_t bFindStringInGsmRxBuffer(char *str);
int16_t bFindCharacterInString(char *str, char character);
void vGsmInitRxBuffer();
uint8_t vGsmTx(char *command, uint8_t *Buf, uint16_t size);
uint8_t vGsmCheckIfAnswerExists(char *answer);
void vGsmSendCommandTo(char *command, uint16_t timeout);
void vGsmSendDataTo(char *command, uint8_t *Buf, uint16_t size, uint16_t timeout);
void vGsmRestart();
uint8_t vServerSendDataTo(xGsmToServerTypeDef *TxData);
uint8_t vServerReceiveDataFrom(uint8_t *Buf, uint16_t *size);
uint8_t vServerCheckConnectionWith();
uint8_t vGsmInit();
uint8_t vGsmInitTcp();


#endif /* GSM_H_ */
