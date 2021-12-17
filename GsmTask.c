
#include "GsmTask.h"

xGsmToServerTypeDef xServerTxDataTo;
extern QueueHandle_t QueueDataToServer;
extern QueueHandle_t QueueDataToVendista;

void vGsmTask(void const *argument)
{
	xDataToServer ToServer;
	xDataToVendista ToVendista;
	uint8_t answer = 0, Rx = 0, Tx = 0, ErrorCounter = 0;
	uint16_t DataSize;
	uint32_t Response;
	vTaskDelay(pdMS_TO_TICKS(1000));
	vGsmStartRx();
	for(;;)
	{
		vTaskDelay(pdMS_TO_TICKS(10));
		if(!vGsmInit())
			continue;
		if(!vGsmInitTcp())
			continue;

		if (Rx)	{ // if it is the situation of receiving data
			if ((answer = vServerReceiveDataFrom(GsmRxBuff, &DataSize))) {
				if (answer == Answer_OK) {
					ToVendista.size = DataSize;
					ToVendista.Buf = GsmRxBuff;
					ErrorCounter = 0;
					xQueueSendToBack(QueueDataToVendista, &ToVendista, 500);
					vLogSendTo(NoCommand, ToVendista.Buf, ToVendista.size);
				}
				else {
					if(ErrorCounter++ < 2) {
						xQueueSendToBack(QueueDataToServer, &ToServer, 500);
						Rx = 0;
						Tx = 0;
						continue;
					}
					vLogSendTo(SERVER_RX_ERROR, 0, 0);
					ErrorCounter = 0;
					ToVendista.size = 0;
				}
				Rx = 0;
				Tx = 0;
			}
			continue;
		}

		if (Tx) { // if it is the situation of transmitting data
			if ((Response = vServerSendDataTo(&xServerTxDataTo)) == Answer_OK )
				Rx = 1;
			else if (Response == Answer_Err)
				Tx = 0;
		}

		else { // if no Rx and no Tx then check your connection with the server
			if ((Response = vServerCheckConnectionWith()) == Answer_OK) {
				if ((Response = xQueuePeek(QueueDataToServer, &ToServer, 10)) == pdPASS)
					if (((xQueueReceive(QueueDataToServer, &ToServer, 10)) == pdPASS) && (ToServer.size)) {
						Tx = 1;
						Rx = 0;
						xServerTxDataTo.size = ToServer.size;
						memcpy(xServerTxDataTo.Buffer, ToServer.Buf, ToServer.size);
						vLogSendTo(0, xServerTxDataTo.Buffer, xServerTxDataTo.size);
					}
			}
			else if (Response == Answer_Err)
				vLogSendTo(MODEM_CONNECT_CLOSED, 0, 0);
		}
	} // for (;;)

} // function end
