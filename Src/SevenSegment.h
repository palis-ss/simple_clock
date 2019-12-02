#ifndef __SevenSegment_H__
#define __SevenSegment_H__




#define OP_NOOP   0
#define OP_DIGIT0 1
#define OP_DIGIT1 2
#define OP_DIGIT2 3
#define OP_DIGIT3 4
#define OP_DIGIT4 5
#define OP_DIGIT5 6
#define OP_DIGIT6 7
#define OP_DIGIT7 8
#define OP_DECODEMODE  9
#define OP_INTENSITY   10
#define OP_SCANLIMIT   11
#define OP_SHUTDOWN    12
#define OP_DISPLAYTEST 15


#include "main.h"
#include "stm32f1xx_hal.h"


struct _SevenSegment_t_
{
	SPI_HandleTypeDef *hspi;
	int numdevices;
};


void SevenSegment_Init(int numdigits, GPIO_TypeDef *port, uint16_t pin, uint8_t brightness, SPI_HandleTypeDef *handle);
void SevenSegment_BlankDisplay(void);
int SevenSegment_IsInitialized(void);
void SevenSegment_SetDigit(uint16_t digit, uint16_t num);
void SevenSegment_SetPeriod(uint16_t digit, int onoff);

#endif
