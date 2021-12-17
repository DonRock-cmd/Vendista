#include "Uart.h"



uint16_t GsmRxBuffCounter = 0;
uint16_t VenRxBuffCounter = 0;



/*
 *@brief Permission to receive data from GSM
 */
void vGsmStartRx(void)
{
	HAL_UART_Receive_IT(&huart3, &GsmInByte, 1);
}

/*
 *@brief Permission to receive data from Vendista
 */
void vVenStartRx(void)
{
	HAL_UART_Receive_IT(&huart2, &VenInByte, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

	if (huart->Instance == USART3)
	{
//		if (GsmRxBuffCounter < sizeof(GsmRxBuff))
			if ((GsmRxBuff[GsmRxBuffCounter++] = GsmInByte) == 0x0A)
				GsmRxTemporaryCounter++;
		HAL_UART_Receive_IT(&huart3, &GsmInByte, 1);
		return;
	}


	if (huart->Instance == USART2) {
		if (VenRxBuffCounter < sizeof(VenRxBuff))
			VenRxBuff[VenRxBuffCounter++] = VenInByte;
		HAL_UART_Receive_IT(&huart2, &VenInByte, 1);
		return;
	}
}
