#include "gsm.h"



static uint16_t GsmTimeout = TimeoutPwr;
static uint8_t GsmCommandMemory = 0;
static uint8_t GsmCommand = 0;
static uint16_t OffsetInData = 0;

/*
 * @brief Find a string in Gsm buffer
 * @para  *str: a pointer to the string, for which we need to perform a search
 * @ret   0: the string could not be found. 1: the string was founded
 */
uint8_t bFindStringInGsmRxBuffer(char *str)
{
	uint16_t i = 0, k = 0;
	int len = strlen(str);
	for(; i<GsmRxBuffCounter; i++) {
		if(GsmRxBuff[i] == str[k]) {
			for(; k<len; k++) {
				if(GsmRxBuff[i+k] != str[k])
					break;
			}
			if (k == len)
				return 1;
		}
		k = 0;
	}
	return 0;
}

/*
 * @brief check if an answer exists in Gsm Receiving buffer
 * @para  *answer: a pointer to the answer, for which we are searching
 * @ret   0: Not found, 1: was Founded, 2: Buffer contains other answers
 *        not the one required, 3: Timeout
 */
uint8_t vGsmCheckIfAnswerExists(char *answer)
{
	if (GsmTimeout-- == 0) {
		GsmCommandMemory = GsmCommand;
		GsmCommand = Gsm_Err;
		return 3;
	}
	if (GsmRxTemporaryCounter>=2 || (GsmRxTemporaryCounter==1 && strlen(answer)==1) ){
		if(bFindStringInGsmRxBuffer(answer)) {
			GsmCommand++;
			return 1;
		}
		else
			return 2;
	}
	return 0;
}

/*
 * @brief Find the position of a character inside a string
 * @para  *str: a pointer to the string, inside which we need to perform the search
 * @para  character: the character, for which we are searching
 * @ret   -1: if the character was not found, otherwise return the position of the character
 */
int16_t bFindCharacterInString(char *str, char character)
{
	uint16_t len = strlen(str), i = 0;
	for(; i<len; i++) {
		if (str[i] == character)
			return i;
	}
	return -1;
}

/*
 * @brief initialize Gsm receiving buffer, which receive data from server
 */
void vGsmInitRxBuffer(){
	GsmRxBuffCounter = 0;
	memset(GsmRxBuff, 0, sizeof(GsmRxBuff));
}

/*
 * @brief Send data, command or (data and command) to Gsm
 * @para  pointer to a string command
 * @para  pointer to the buffer
 * @para  Size of the buffer
 * @ret   1: Successfully send, 0: unable to send;
 */
uint8_t vGsmTx(char *command, uint8_t *Buf, uint16_t size)
{
	uint8_t ComSize = strlen(command);
	uint16_t PacketSize = ComSize + 2*(ComSize>0?1:0);
	uint8_t *PacketBuff = 0;

	PacketSize += size;
	PacketBuff = pvPortMalloc(PacketSize);
	if(PacketBuff) {
		strcpy((char *)PacketBuff, command);
		if(size)
			memcpy(PacketBuff + ComSize, Buf, size);
		if(ComSize)
			*(uint16_t *)&PacketBuff[size-2] = 0x0D0A; //0x0D0A stands for \r\n
		vGsmInitRxBuffer();
		if(HAL_UART_Transmit(&huart3, PacketBuff, PacketSize, TimeoutTx) == HAL_OK) {
			vPortFree(PacketBuff);
			return 1;
		}
	}
	vPortFree(PacketBuff);
	return 0;
}

/*
 * @brief Send command to GSM
 * @para  pointer to the command
 * @para  timeout for sending
 */
void vGsmSendCommandTo(char *command, uint16_t timeout)
{
	if(vGsmTx(command, 0, 0)) {
		GsmTimeout = timeout;
		GsmCommand++;
		return;
	}
	return;
}

/*
 * @brief Send data with command to GSM
 * @para  pointer to the command
 * @para  pointer to data
 * @para  Data size
 * @para  timeout for sending
 */
void vGsmSendDataTo(char *command, uint8_t *Buf, uint16_t size, uint16_t timeout)
{
	if(vGsmTx(command, Buf, size)) {
		GsmTimeout = timeout;
		GsmCommand++;
		return;
	}
	return;
}

/*
 * @brief Restart GSM modem
 */
void vGsmRestart()
{
	GsmCommand = 0;
	GsmTimeout = TimeoutPwr;
	memset((uint8_t *)&xGsmState, 0, sizeof(xGsmState));
	vGsmSendCommandTo("AT+CFUN=1,1", TimeoutAnswer);
	return;
}

/*
 * @brief Send data to server via gsm
 * @para  A structure for that data type, it's elements are pointer to buffer and size of the buffer
 * @ret   0: Sending is going on, 1: Successfully sent, 2: Error in sending
 */
uint8_t vServerSendDataTo(xGsmToServerTypeDef *TxData)
{
	uint8_t buf[2] = {0};
	switch (GsmCommand) {
	case 0:
		sprintf((char *)buf, "%d", TxData->size);
		// "AT+CIPSEND=" request to transfer n bytes
		vGsmSendDataTo("AT+CIPSEND=", TxData->Buffer, TxData->size, TimeoutAnswer);
		break;
	case 1:
		vGsmCheckIfAnswerExists(">");
		break;
	case 2:
		vGsmSendDataTo("\0", TxData->Buffer, TxData->size, TimeoutDataIn);
		break;
	case 3:
		if(vGsmCheckIfAnswerExists("SEND OK") == 1) {
			GsmRxTemporaryCounter = 0;
			TxData->size = 0;
			GsmTimeout = TimeoutDataIn;
			GsmCommand = 0;
			return 1;
		}
		break;
	default: // Sending error
		GsmCommand = 0;
		xGsmState.net = 0;
		return 2;
	}
	return 0;
}

/*
 * @brief Receiving data from the server
 * @para  A pointer to the receiving buffer
 * @para  Size of data that will be received
 * @ret   0: Receiving is going on, 1: Successfully received, 2: Error in Receiving
 */
uint8_t vServerReceiveDataFrom(uint8_t *Buf, uint16_t *size)
{
	uint16_t response = 0, offset = 0, end = 0;
	switch(GsmCommand) {
	case 0:
		response = vGsmCheckIfAnswerExists("+CIPRXGET: 1");
		if(response) {
			if (response == 1) {
				vGsmSendCommandTo("AT+CIPRXGET=2,1024", TimeoutDataIn);
				GsmCommand = 1; // because it was increased twice in "vGsmSendCommand" and "vGsmCheckIfAnswerExists"
			}
				else if (bFindStringInGsmRxBuffer("CLOSED") ) {
				xGsmState.net = 0;
				return 2;
			}
		}
		break;
	case 1:
		if(vGsmCheckIfAnswerExists("+CIPRXGET: 2") == 1) {
			if((offset = bFindCharacterInString((char *)GsmRxBuff, ',')) > 0) {
				response = atoi((const char *)GsmRxBuff + offset + 1); /* depending on the size of "response"
				 which is right now 2, "atoi" takes two bytes from the memory and convert them to integer,
				  the start address is the first parameter to atoi. if response was "uint32_t" then atoi
				  takes 4 bytes and convert them to integer.*/
				end = bFindCharacterInString((char *)GsmRxBuff, '\n') + 2 + 1;
				memcpy(Buf + OffsetInData, GsmRxBuff + end, response);
				OffsetInData += response;
				if((offset += bFindCharacterInString((char *) GsmRxBuff + offset + 1, ',')) > 0) {
					if ((response = atoi((char*) GsmRxBuff + offset + 1)) != 0) {
						vGsmSendCommandTo("AT", TimeoutAnswer);
						GsmCommand = 0;
						return 0;
					}
				}
				GsmCommand = 0;
				*size = OffsetInData;
				OffsetInData = 0;
#ifdef CloseConnect
				vGsmSendCommandTo("AT+CIPCLOSE", TimeoutAnswer);
				GsmCommand = 0;
#endif
				return 1;
			}
		}
		break;
	default:
#ifdef CloseConnect
		vGsmSendCommandTo("AT+CIPCLOSE", TimeoutAnswer);
#else
		vGsmInitRxBuffer();
#endif
		GsmCommand = 0;
		GsmTimeout = 0;
		return 2;
	}
	return 0;
}

/*
 * @brief Check connection with server
 * @ret   0: checking is going on, 1: connected, 2: Couldn't connect
 */
uint8_t CheckNum = 0;
uint32_t TimeoutForConnection = 0;
uint8_t vServerCheckConnectionWith()
{
	if ( TimeoutForConnection++ < TimeoutCheckConnection )
		return 0;

	if(CheckNum == 0){
		CheckNum = 1;
		vGsmTx("AT+CIPSTATUS", 0, 0); // get connection status
	}
	else {
		if(GsmRxTemporaryCounter >= 2) {
			CheckNum = 0;
			if(bFindStringInGsmRxBuffer("CONNECT OK")) {
				TimeoutForConnection = 0;
				return 1;
			}
			else {
				xGsmState.net = 0;
				TimeoutForConnection = 0;
				CheckNum = 0;
				return 2;
			}
		}
		else {
			if (TimeoutForConnection > TimeoutCheckConnection * 2) {
			xGsmState.net = 0;
			TimeoutForConnection = 0;
			CheckNum = 0;
			return 2;
			}
		}
	}
	return 0;
}

/*
 * @brief Initialization of the modem, Registration at GSM network
 * @ret   1: Registered successfully, 0: Couldn't register
 */
uint8_t repetition = 0;
uint8_t vGsmInit()
{
	uint8_t response = 0;

	if (xGsmState.reg)
		return 1;

	switch (GsmCommand){
	case 0:
		if(GsmTimeout--)
			return 0;
		GsmTimeout = TimeoutPwr;
		GsmCommand++;
		repetition = 0;
		break;

	case 1:
		vGsmSendCommandTo("AT", TimeoutAnswer);
		xGsmState.connect = 0;
		break;

	case 2:
		if(vGsmCheckIfAnswerExists("OK")) {
			xGsmState.connect = 1;
			vLogSendTo(MODEM_CONNECT_OK, 0, 0);
		}
		if(GsmCommand == Gsm_Err)
			if (repetition++ < Repeat_AT_command) {
				GsmCommand = 1;
				break;
			}
		vLogSendTo(MODEM_CONNECT_ERROR, 0, 0);

	case 3:
		vGsmSendCommandTo("ATE0&W", TimeoutAnswer);
		break;

	case 4:
		vGsmCheckIfAnswerExists("OK");
		break;

	case 5:
		vGsmSendCommandTo("AT+CSMINS?", TimeoutAnswer);
		break;

	case 6:
		if(vGsmCheckIfAnswerExists("+CSMINS: 0,1") == 1){
			xGsmState.sim = 1;
			vLogSendTo(SIM_OK, 0, 0);
		}
		if(GsmCommand == Gsm_Err) {
			xGsmState.errors |= GsmErrorSim;
			vLogSendTo(SIM_ERROR, 0, 0);
		}
		break;

	case 7:
		vGsmSendCommandTo("AT+CFUN=1", TimeoutAnswer);
		break;

	case 8:
		vGsmCheckIfAnswerExists("OK");
		break;

	case 9:
		vGsmSendCommandTo("AT+CPIN?", TimeoutAnswer);
		break;

	case 10:
		vGsmCheckIfAnswerExists("+CPIN: READY");
		if (GsmCommand == Gsm_Err)
			xGsmState.errors |= GsmErrorSim;
		break;

	case 11:
		vGsmSendCommandTo("AT+CIPRXGET=1", TimeoutAnswer);
		break;

	case 12:
		vGsmCheckIfAnswerExists("OK");
		break;

	case 13:
		vGsmSendCommandTo("AT+CREG?", TimeoutAnswer);
		break;

	case 14:
		if( (response = vGsmCheckIfAnswerExists("+CREG: 0,1")) == 1)
			GsmTimeout = TimeoutPwr;
		if(response == 2)
			GsmCommand = 13;
		break;

	case 15:
		if(GsmTimeout--)
			return 0;
		GsmCommand++;
		break;

	case 16:
		xGsmState.reg = 1;
		GsmCommand = 0;
		xGsmState.net = 0;
		vLogSendTo(GSM_OK, 0, 0);
		return 1;
		break;

	default:
		if (GsmCommandMemory == 14)
			vLogSendTo(GSM_ERROR, 0, 0);
		else
			vLogSendTo(MODEM_RESTART, 0, 0);
		vGsmRestart();
		GsmCommand = 0;
		GsmTimeout = TimeoutPwr;
		break;

	}
	return 0;
}

/*
 * @brief establishing a connection with the server
 * @ret   1: Connection established, 0: Failed to establish a connection
 */
static uint8_t RepeatCounter = 0;
uint8_t vGsmInitTcp()
{
	if (xGsmState.net)
		return 1;

	switch(GsmCommand){
	case 0:
		vGsmSendCommandTo("AT+CGATT=1", TimeoutCGATT);
		break;

	case 1:
		vGsmCheckIfAnswerExists("OK");
		break;

	case 2:
		vGsmSendCommandTo("AT+CIPSHUT", TimeoutAnswer*10);
		break;

	case 3:
		vGsmCheckIfAnswerExists("SHUT OK");
		break;

	case 4:
		vGsmSendCommandTo("AT+CIPSTATUS", TCPTimeout);
		break;

	case 5:
		if( vGsmCheckIfAnswerExists("STATE: IP INITIAL") == 2 ){
			if (vGsmCheckIfAnswerExists("CONNECT OK") == 1) {
				xGsmState.net = 1;
				GsmCommand = 0;
				RepeatCounter = 0;
			}
			else
				GsmCommand = 0;
		}
		break;

	case 6:
		vGsmSendCommandTo("AT+CIPMUX=0", TimeoutAnswer);
		break;

	case 7:
		vGsmCheckIfAnswerExists("OK");
		break;

	case 8:
		vGsmSendCommandTo("AT+CIPCSGP=1,\"internet.beeline.ru\",\"beeline\",\"beeline\"", TimeoutAnswer*4);
		break;

	case 9:
		vGsmCheckIfAnswerExists("OK");
		break;

	case 10:
		vGsmSendCommandTo("AT+CIPSTART=\"TCP\",\"slave.ifuture.su\",\"88\"", TCPTimeout);
		break;

	case 11:
		if ( vGsmCheckIfAnswerExists("CONNECT OK") == 1 ) {
			vGsmInitRxBuffer();
			xGsmState.net = 1;
			GsmCommand = 0;
			RepeatCounter = 0;
			vLogSendTo(SERVER_CONNECT_OK, 0, 0);
			return 1;
		}
		break;

	default:
		vGsmRestart();
		vLogSendTo(SERVER_CONNECT_ERROR, 0, 0);
		break;
	}
	return 0;
}


































