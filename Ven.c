#include "Ven.h"


uint32_t VenRxTimeout = 0;

void ReadCardTask(uint32_t sum)
{
	uint8_t data[11];
	*(uint32_t *) data = sum;
	*(uint16_t *) &data[4] = 0x4306;
	*(uint32_t *) &data[6] = 0;
	data[10] = 0;
	Tx2Ven(data, 11, ReadCard);
}

//void ShowPictureTask(uint8_t PicNum)
//{
//	uint8_t data = PicNum;
//	Tx2Ven(&data, 1, ShowPicture);
//}

/*
 *@brief Show QR code on terminal screen
 *@para  *QR: pointer to QR code
 *@para  QR_length: size of QR code
 *@para  *Title: pointer to QR title
 *@para  Title_length: size of QR title
 */
void ShowQRTask(uint8_t *QR, uint8_t QR_length, uint8_t *Title, uint8_t Title_length)
{
	uint8_t *puf;
	uint16_t size = 2 + QR_length + Title_length;
	if ( (puf = pvPortMalloc(size)) ) {
		puf[0] = QR_length;
		memcpy(puf+1, QR, QR_length);
		puf[QR_length+1] = Title_length;
		memcpy(puf+ 2 + QR_length, Title, Title_length);
		Tx2Ven(puf, size, ShowQR);
	}
	else
		vLogSendTo(MASTR_MEMORY_ERROR, 0, 0);
	vPortFree(puf);
}

//void RebootTask()
//{
//	Tx2Ven(NULL, 0, Reboot);
//}
//
//void CancelLastTransactionTask()
//{
//	Tx2Ven(NULL, 0, CancelLastTransaction);
//}
//
//void CancelReadCardTask()
//{
//	Tx2Ven(NULL, 0, CancelReadCard);
//}

void FillScreenTask(enum Colors col)
{
	uint8_t puf[2];
	uint16_t color = GetHexRepresentationOfColor(col);
	puf[0] = (uint8_t)color;
	puf[1] = color >> 8;
	Tx2Ven(puf, 2, FillScreen);
}

void WriteLineTask(enum Colors BackgroundColor, enum Colors TextColor,\
		enum Fonts f, uint16_t x, uint16_t y, uint8_t strlength, char *str)
{
	uint8_t *puf = NULL;
	if ( (puf = pvPortMalloc(strlength+10)) ) {
		*(uint16_t *) puf = GetHexRepresentationOfColor(BackgroundColor);
		*(uint16_t *) &puf[2] = GetHexRepresentationOfColor(TextColor);
		puf[4] = f;
		*(uint16_t *) &puf[5] = x;
		*(uint16_t *) &puf[7] = y;
		puf[9] = strlength;
	//	memcpy(puf+10, str, strlength);
		Convert_UTF8_To_Win1251((char *)str, (char *)puf + 10);
		Tx2Ven(puf, strlength+10, WriteLine);
	}
	else
		vLogSendTo(MASTR_MEMORY_ERROR, 0, 0);
	vPortFree(puf);
}

/*void ServerConnectStateTask(uint8_t SignalStrength, uint8_t NameLen,\
							uint8_t *Name)
{
	uint8_t *puf = NULL;
	puf = pvPortMalloc(NameLen+2);
	puf[0] = SignalStrength;
	puf[1] = NameLen;
	memcpy(puf+2, Name, NameLen);
	Tx2Ven(puf, NameLen+2, ServerConnectionState);
	vPortFree(puf);
}

void SetCustomValueTask(uint32_t value)
{
	Tx2Ven((uint8_t *) &value, 4, SetCustomValue);
}*/

uint16_t GetHexRepresentationOfColor(enum Colors color)
{
	const uint16_t H[] ={0x001F, 0xF800, 0x07E0, 0x0000, 0xFFFF, 0xFFE0};
	return H[color];
}

void Tx2Ven(uint8_t *puf, uint16_t DataSize, enum CommandType cmd)
{
	uint8_t *UART_Buf;
	UART_Buf = pvPortMalloc(DataSize+5);
	UART_Buf[4] = cmd;
	*(uint16_t *) &UART_Buf[2] = GetCRC16((const uint8_t *)(UART_Buf+4), DataSize+1);
	*(uint16_t *) &UART_Buf[0] = DataSize + 1;  // '1' is commandType
	memcpy(UART_Buf+5, puf, DataSize);
	HAL_UART_Transmit(&huart1, (uint8_t *)UART_Buf, DataSize+5, 1000);
}

/*
 А	0xC0	К	0xCA	Ф	0xD4	Ю	0xDE	и	0xE8	т	0xF2	ь	0xFC
 Б	0xC1	Л	0xCB	Х	0xD5	Я	0xDF	й	0xE9	у	0xF3	э	0xFD
 В	0xC2	М	0xCC	Ц	0xD6	а	0xE0	к	0xEA	ф	0xF4	ю	0xFE
 Г	0xC3	Н	0xCD	Ч	0xD7	б	0xE1	л	0xEB	х	0xF5	я	0xFF
 Д	0xC4	О	0xCE	Ш	0xD8	в	0xE2	м	0xEC	ц	0xF6
 Е	0xC5	П	0xCF	Щ	0xD9	г	0xE3	н	0xED	ч	0xF7	Ё	0xA8
 Ж	0xC6	Р	0xD0	Ъ	0xDA	д	0xE4	о	0xEE	ш	0xF8	ё	0xB8
 З	0xC7	С	0xD1	Ы	0xDB	е	0xE5	п	0xEF	щ	0xF9
 И	0xC8	Т	0xD2	Ь	0xDC	ж	0xE6	р	0xF0	ъ	0xFA
 Й	0xC9	У	0xD3	Э	0xDD	з	0xE7	с	0xF1	ы	0xFB
 *
 */
/*
 *@brief convert UTF8 to Win1251
 *@para  *in: a pointer for input string
 *@para  *out: a pointer for output string
 */
void Convert_UTF8_To_Win1251(char *in, char *out)
{
	uint8_t temp;
	uint16_t j = 0;
	for (uint16_t i = 0; i < 255; i++, j++) {
		if (in[i] < 128) {
			if ((out[j] = in[i]) == 0)
				break;
		} else {
			temp = in[i + 1];
			if (in[i] == 0xd0) {
				out[j] = temp - 0x90 + 0xC0;
			} else {
				out[j] = temp - 0x80 + 0xF0;
			}
			i++;
		}
	}
}


/*
 *@brief Check if there is data, received from the terminal, if data has stopped coming
 * 		 during WAIT_DATA_TIME, it is considered that the packet has been accepted
 *@ret   0: did not finish receiving yet, otherwise the number of received bytes
 */
#define WAIT_DATA_Time 30
static uint16_t WaitCounter = 0, VenRxBuffCounterMemory = 0;

uint16_t VenCheckIfaPacketRecieved(void)
{
	if(VenRxBuffCounter) { // There are received data
		if (VenRxTimeout++ > VEN_RX_TIMEOUT) {
			VenRxTimeout = 0;
			WaitCounter = 0;
			VenRxBuffCounterMemory = 0;
			VenRxBuffCounter = 0;
		}
		if (VenRxBuffCounterMemory == VenRxBuffCounter) {
			if (WaitCounter++ > WAIT_DATA_Time && VenRxBuffCounter > 4) {
			WaitCounter = 0;
			VenRxBuffCounterMemory = 0;
			return VenRxBuffCounter;
			}
		}
		else {
			VenRxBuffCounterMemory = VenRxBuffCounter;
			WaitCounter = 0;
		}
	}
	return 0;
}


/*
 *@brief parsing received packets from the terminal; If more than one packet was
 *			received then it returns the first packet in the buffer.
 *@para  *VenState: a pointer to a structure object which carry packet information
 *@ret	 0: error in parsing, 1: successfully parsed, 2:  successfully parsed, but packets are there in the buffer
 */
xVenStateTypeDef xVenState;

uint8_t VenParseReceivedData(xVenStateTypeDef *VenState)
{
	uint16_t NumOfBytesReceived = VenRxBuffCounter;
	uint16_t DataSize = *(uint16_t *)VenRxBuff;
	uint16_t crc = GetCRC16(VenRxBuff + 4, DataSize);
	uint16_t PacketSize = DataSize + 4;
	if ( (crc != *(uint16_t *)&VenRxBuff[2]) || NumOfBytesReceived<5 || DataSize>255) {
		VenRxBuffCounter = 0;
		return 0;
	}

	VenState->command = VenRxBuff[4];
	VenState->size = DataSize - 1;
	memcpy(VenState->puf, VenRxBuff + 5, DataSize-1);
	if (PacketSize+4+1 < VenRxBuffCounter) { // there is more packets in the buffer
		VenRxBuffCounter -= PacketSize;
		memcpy(VenRxBuff, VenRxBuff+PacketSize, VenRxBuffCounter);
		return 2;
	}
	VenRxBuffCounter = 0;
	return 1;
}









