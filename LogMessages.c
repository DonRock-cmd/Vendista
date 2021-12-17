
#include <LogMessages.h>



static char LogTxBuf[800];

/*
 @brief Convert data and store them as Hex string
 @param *in - pointer data
 @param  *out - output conversion result
 @param size - size of data
 */
void bConvertStringToHex(uint8_t* in, uint8_t* out, uint16_t size)
{
	int offset = 0;
	for (int i = 0; i < size; i++) {
		sprintf((char*)out + offset, "%02x ", in[i]);
		offset += 3;
	}
}

void vLogSendTo(uint8_t com, uint8_t *buf, uint8_t size)
{
#ifdef LogOn
	uint8_t type = 0, ComMemory = com;
	char *p;

	memset(LogTxBuf, 0, sizeof(LogTxBuf));
	if (com >= GSM_OK) {
		type = 4;
		p = (char *)&Log_GsmState[com - GSM_OK][0];
	}
	else if (com >= NoCommand) {
		if (com == NoCommand)
			type = 3;
		else {
			type = 1;
			com -= 0x11;
		}
		p = (char *)&Log_TxtOut[com][0];
	}
	else {
		if (com == 0)
			type = 2;
		else
			com--;
		p = (char *)&Log_TxtIn[com][0];
	}

	sprintf((char *)LogTxBuf, "%d. %s", (int)bGetTimeInMs(), LogType[type]);
	if (size)
		bConvertStringToHex(buf, (uint8_t*)LogTxBuf + strlen(LogTxBuf), size);
	if (ComMemory != 0 && ComMemory != NoCommand)
		sprintf((char *) LogTxBuf + strlen(LogTxBuf), " %s", p);

	size = strlen(LogTxBuf);
	*(uint16_t *) &LogTxBuf[size++] = 0x0D0A; // stands for \r\n

	if (HAL_UART_Transmit(&huart1, (uint8_t*)LogTxBuf, size, TimeoutTx) == HAL_OK)
		return;

#endif
}
