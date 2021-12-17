

#ifndef INC_GSMTASK_H_
#define INC_GSMTASK_H_

#include "main.h"
#include "LogMessages.h"
#include "gsm.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "Uart.h"


typedef struct{
	uint8_t *Buf;
	int16_t size;
}
xDataToServer;

typedef struct{
	uint8_t com;
	uint8_t *Buf;
	uint16_t size;
}
xDataToVendista;

void vGsmTask(void const *argument);

#endif /* INC_GSMTASK_H_ */
