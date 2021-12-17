
#ifndef INC_VEN_H_
#define INC_VEN_H_


#include "main.h"
#include "LogMessages.h"
#include "gsm.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "Uart.h"

#define VenBuffSize 			270

enum Colors {Blue,   Red,    Green,  Black,  White,  Yellow};
//           0x001F  0xF800  0x07E0  0x0000  0xFFFF  0xFFE0
enum CommandType {NU1, ReadCard, PacketFromServer, ShowPicture, ShowQR,\
	             Reboot, PingServer, CancelLastTransaction, CancelReadCard,\
				 FillScreen, WriteLine, ServerConnectionState, SetCustomValue,\
				 demo1,demo2,NU2,NU3, Touch, PacketToServer, CardReadResult,\
				 CardAuthorizationResult, Ack, RebootReport}; // NU: not Used
enum Fonts {Small, Large, Narrow, Wide};

typedef struct
{
	enum CommandType command;
	uint16_t size;
	uint8_t  puf[VenBuffSize];
}
xVenStateTypeDef;
extern xVenStateTypeDef xVenState;


#define VEN_RX_TIMEOUT 500 // 0,5 sec
#define VEN_TX_TIMEOUT 500

void Tx2Ven(uint8_t *puf, uint16_t DataSize, enum CommandType cmd);
uint16_t GetHexRepresentationOfColor(enum Colors color);
void ReadCardTask(uint32_t sum);
void FillScreenTask(enum Colors col);
void WriteLineTask(enum Colors BackgroundColor, enum Colors TextColor,\
		enum Fonts f, uint16_t x, uint16_t y, uint8_t strlen, char *str);
void ShowQRTask(uint8_t *QR, uint8_t QR_length, uint8_t *Title, uint8_t Title_length);
void Convert_UTF8_To_Win1251(char *in, char *out);
uint16_t VenCheckIfaPacketRecieved(void);
uint8_t VenParseReceivedData(xVenStateTypeDef *VenState);


#endif /* INC_VEN_H_ */
