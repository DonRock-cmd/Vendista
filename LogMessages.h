
#ifndef INC_LOGMESSAGES_H_
#define INC_LOGMESSAGES_H_



#include "main.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "gsm.h"
#include "Ven.h"
#include "Uart.h"
//
#define LogOn
//
#define GSM_OK	   200
#define GSM_ERROR  201
#define SIM_OK     		202
#define SIM_ERROR  		203
#define SERVER_CONNECT_OK    204
#define SERVER_CONNECT_ERROR 205
#define MODEM_CONNECT_OK     	206
#define MODEM_CONNECT_ERROR  	207
#define MODEM_RESTART        	208
#define MODEM_CONNECT_CLOSED 	209
#define CARD_READ_ERROR      		210
#define CARD_NOT_FOUND       		211
#define CARD_READ_OK         		212
#define SERVER_RX_ERROR      			213
#define TERMINAL_RX_ERROR    				214
#define TERMINAL_TX_ERROR    				215
#define MASTR_MEMORY_ERROR   					216

#define NoCommand            0x10
extern UART_HandleTypeDef huart1;


static const char LogType[5][4]={{"> "}, {"< "}, {">S "}, {"<S "}, {" "}};
static const char Log_TxtIn[12][22] = {
		{"ReadCard"},  // 1
		{"PacketFromServer"}, //2
		{"ShowPicture"}, //3
		{"ShowQR"}, //4
		{"Reboot"}, //5
		{"PingServer"}, //6
		{"CancelLastTransaction"}, //7
		{"CancelReadCard"}, //8
		{"FillScreen"}, //9
		{"WriteLine"}, //10
		{"ServerConnectState"}, //11
		{"SetCustomValue"} //12
};
static const char Log_TxtOut[6][24] = {
		{"Touch"}, //0x11
		{"PacketToServer"}, // 0x12
		{"CardReadResult"}, //0x13
		{"CardAuthorizationResult"}, //0x14
		{"Ack"}, //0x15
		{"RebootReport"} //0x16
};

static const char Log_GsmState[17][24] = {
		{"GSM REG OK"},
		{"GSM REG ERROR"},
		{"SIM OK"},
		{"SIM ERROR"},
		{"Server connect OK"},
		{"Server connect ERROR"},
		{"Modem connect OK"},
		{"Modem connect ERROR"},
		{"Modem RESTART"},
		{"Server connect CLOSED"},
		{"Card reading ERROR"},
		{"Card not found"},
		{"Card reading OK"},
		{"Server receive ERROR"},
		{"Terminal receive ERROR"},
		{"Terminal transmit ERROR"},
		{"Memory allocation ERROR"}
};

void bConvertStringToHex(uint8_t* in, uint8_t* out, uint16_t size);
void vLogSendTo(uint8_t com, uint8_t *buf, uint8_t size);



#endif /* INC_LOGMESSAGES_H_ */
