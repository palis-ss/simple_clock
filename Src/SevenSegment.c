/* LED Matrix  */


#include "SevenSegment.h"


int SevenSegmentInitialized = 0;

// hardware
GPIO_TypeDef *SPICSPort;
uint16_t SPICSPin;
SPI_HandleTypeDef *hspi;

int Digit[8] = {0};
int Period[8] = {0};
int NumDigits = 0;


void SPITransmitData(uint16_t data)
{				
	HAL_GPIO_WritePin(SPICSPort, SPICSPin, GPIO_PIN_RESET);	
	HAL_SPI_Transmit(hspi, (uint8_t *)&data, 1, 1);	
	HAL_GPIO_WritePin(SPICSPort, SPICSPin, GPIO_PIN_SET);
	
	HAL_Delay(1);
}


void SevenSegment_Init(int numdigits, GPIO_TypeDef *port, uint16_t pin, uint8_t brightness, SPI_HandleTypeDef *handle)
{
	SPICSPort = port;
	SPICSPin = pin;
	hspi = handle;
	
			
	//allocating display buffer
	SevenSegmentInitialized = 0;
	NumDigits = numdigits;
		
	SPITransmitData(0xF00);  // normal operation
	SPITransmitData(0x900 | ((1<<NumDigits) - 1));  // decode N digits
	
	SPITransmitData(0xA00 | brightness);  // set intensity
	SPITransmitData(0xB07);  // max scan limit	
	SPITransmitData(0xC01);  // turn on
	
	SevenSegmentInitialized = 1;
	SevenSegment_BlankDisplay();
}


void SevenSegment_BlankDisplay(void)
{
	uint16_t i;
	
	if(SevenSegmentInitialized == 0)
		return;
	
	for(i=0; i<8; i++)
		SPITransmitData(((i+1)<<8)|0x0F);  // blank digit i
}

int SevenSegment_IsInitialized(void)
{
	return SevenSegmentInitialized;
}

void SevenSegment_DisplayNumber(uint16_t digit)
{
	if(SevenSegmentInitialized == 0)
		return;
	
	SPITransmitData(((digit+1)<<8)| (Digit[digit] | (Period[digit] << 7)));
}

void SevenSegment_SetDigit(uint16_t digit, uint16_t num)
{
	Digit[digit] = num;
	SPITransmitData(((digit+1)<<8)| (Digit[digit] | (Period[digit] << 7)));
}

void SevenSegment_SetPeriod(uint16_t digit, int onoff)
{	
	Period[digit] = onoff;
	SPITransmitData(((digit+1)<<8)| (Digit[digit] | (Period[digit] << 7)));
}
////////////////////////////////////////////////////////////////////
