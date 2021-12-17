
#include "MainTask.h"

xGsmStateTypeDef xGsmState;
xMasterStateTypeDef xMasterState;

extern uint8_t VenTxBuff[];
extern uint8_t VenRxBuff[];
extern QueueHandle_t QueueTasks;
extern QueueHandle_t QueueDataToServer;
extern QueueHandle_t QueueDataToVendista;
extern uint8_t DoesOneSecondPassed;

#define NUM_PICTURE_PUSH   1 //номер картинки "Нажмите кнопку для оплаты картой"


void vMainTask(void const *argument)
{
	xGsmToServerTypeDef DataToServer;
	xDataToVendista xVenTxDataTo;
	uint8_t Task, Response;
	uint8_t TimeCounter = 4;
	uint8_t TimeoutPing =  TIME_SEND_PING;
	vTaskDelay(pdMS_TO_TICKS(3000));
	vVenStartRx();
	Task = FillScreen;
	xQueueSendToBack(QueueTasks, &Task, 500);
	for(;;)
	{
		if (DoesOneSecondPassed) {

			DoesOneSecondPassed = 0;
			if (TimeCounter)
				if (--TimeCounter == 0)
					xQueueSendToBack(QueueTasks, (uint8_t *)ShowPicture, 500);

			if (xMasterState.CommandTimeout)
				if(--xMasterState.CommandTimeout == 0) {
					if(xMasterState.command == ReadCard)
						xQueueSendToBack(QueueTasks, (uint8_t *)CancelLastTransaction, 500);
					if(xMasterState.command == FillScreen)
						xQueueSendToBack(QueueTasks, (uint8_t *)FillScreen, 500);
					xMasterState.command = 0;
				}

			if (TimeoutPing-- == 0) {
				TimeoutPing = TIME_SEND_PING;
				xQueueSendToBack(QueueTasks, (uint8_t *)PingServer, 500);
			}

		}

		if (xMasterState.command == 0 && xQueuePeek(QueueTasks, &Task, 0)==pdPASS ) {
			Response = xQueueReceive(QueueTasks, &Task, 0);
			if (Response == pdPASS && Task)
				switch(Task) {
				case ReadCard:
					ReadCardTask(100);
					xMasterState.CommandTimeout = TIMEOUT_DATA_CARD_TO_SERVER;
					xMasterState.command = ReadCard;
					TimeCounter = 35;
					break;

				case PingServer:
					Tx2Ven(0, 0, PingServer);
					xMasterState.CommandTimeout = TIMEOUT_PING;
					xMasterState.command = PingServer;
					break;

				case FillScreen:
					FillScreenTask(Blue);
					xMasterState.CommandTimeout = TIMEOUT_COMMAND;
					xMasterState.command = FillScreen;
					break;

				case demo1: ;
					char *txt1 = "   ДЕМО-пример";
					WriteLineTask(White, Red, Wide, 15, 50, sizeof(txt1), txt1);
					xMasterState.CommandTimeout = TIMEOUT_COMMAND;
					xMasterState.command = demo1;
					break;

				case demo2: ;
					char *txt2 = "   Slave режима";
					WriteLineTask(White, Red, Wide, 15, 50, sizeof(txt2), txt2);
					xMasterState.CommandTimeout = TIMEOUT_COMMAND;
					xMasterState.command = demo2;
					break;

//				case WriteLine: ;
//					char *txt2 = "   Slave режима";
//					WriteLineTask(White, Red, Wide, 15, 50, sizeof(txt2), txt2);
//					xMasterState.CommandTimeout = TIMEOUT_COMMAND;
//					xMasterState.command = demo2;
//					break;

				case ShowPicture:
					VenTxBuff[0] = NUM_PICTURE_PUSH;
					Tx2Ven(VenTxBuff, 1, ShowPicture);
					xMasterState.CommandTimeout = TIMEOUT_COMMAND;
					xMasterState.command = ShowPicture;
					break;

				case CancelLastTransaction:
					Tx2Ven(0, 0, CancelLastTransaction);
					xMasterState.CommandTimeout = TIMEOUT_PING;
					xMasterState.command = CancelLastTransaction;
					break;

				case CancelReadCard:
					Tx2Ven(0, 0, CancelReadCard);
					xMasterState.CommandTimeout = TIMEOUT_COMMAND;
					xMasterState.command = CancelReadCard;
					break;

				case ShowQR:
					ShowQRTask((uint8_t *)QrCode, sizeof(QrCode), (uint8_t *)QrText, sizeof(QrText));
					xMasterState.CommandTimeout = TIMEOUT_COMMAND;
					xMasterState.command = ShowQR;
					TimeCounter = 10; // we will start updating the screen in 10 seconds
					break;

				} // switch
		} // the main if

		if (VenCheckIfaPacketRecieved() != 0) {
			if (VenParseReceivedData(&xVenState)) {
				vLogSendTo(xVenState.command, xVenState.puf, xVenState.size);
				uint8_t cmd = xVenState.command;
				switch (cmd) {

				case Touch:
					if (xMasterState.command == 0)
						xQueueSendToBack(QueueTasks, (uint8_t *)ReadCard, 500);

					break;

				case PacketToServer:
					DataToServer.size = xVenState.size;
					DataToServer.Buffer = xVenState.puf;
					xQueueSendToBack(QueueDataToServer, &DataToServer, 500);
					continue;

				case CardReadResult:
					if (xVenState.puf[0] == 0)
						vLogSendTo(CARD_READ_ERROR, 0, 0);
					if (xVenState.puf[0] == 1)
						vLogSendTo(CARD_NOT_FOUND, 0, 0);
					if (xVenState.puf[0] == 2) {
						vLogSendTo(CARD_READ_OK, 0, 0);
						continue;
					}
					break;

				case Ack:
					if (xMasterState.command == PingServer)
						continue;
					if (xMasterState.command == ReadCard)
						continue;
					if (xMasterState.command == FillScreen)
						xQueueSendToBack(QueueTasks, (uint8_t *) demo1, 500);
					if (xMasterState.command == PingServer)
						xQueueSendToBack(QueueTasks, (uint8_t *) demo2, 500);
					break;

				case RebootReport:
					break;

				}// end of switch
			} // end of if (VenParseReceivedData(&xStateVen))
			else {
				vLogSendTo(TERMINAL_RX_ERROR, 0, 0);
			}
			xMasterState.command = 0;
			xMasterState.CommandTimeout = 0;
		}// end of if (VenCheckIfaPacketRecieved() != 0)

		if ( (Response = xQueuePeek(QueueDataToVendista, &xVenTxDataTo, 0)) == pdPASS)
			if ( (Response = xQueueReceive(QueueDataToVendista, &xVenTxDataTo, 0)) == pdPASS) {
				memcpy(VenTxBuff, xVenTxDataTo.Buf, xVenTxDataTo.size);
				Tx2Ven(VenTxBuff, xVenTxDataTo.size, PacketFromServer);
				switch(xMasterState.command) {

					case PingServer:
						break;

					case ReadCard:
						xQueueSendToBack(QueueTasks, (uint8_t *)ShowQR, 500);
						break;

					case CancelLastTransaction:
						xQueueSendToBack(QueueTasks, (uint8_t *)CancelReadCard, 500);
						break;
				}
				xMasterState.CommandTimeout = 0;
				xMasterState.command = 0;
		} // end of if


	} // fro(;;)
}





















