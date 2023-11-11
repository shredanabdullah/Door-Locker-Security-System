/******************************************************************************
 *
 * Module: UART
 *
 * File Name: uart.h
 *
 * Description: Header file for the UART AVR driver
 *
 * Author: Mohamed Tarek
 *
 *******************************************************************************/

#ifndef UART_H_
#define UART_H_

#include "std_types.h"


/*******************************************************************************
 *                               Types Declaration                             *
 *******************************************************************************/
typedef enum
{
	FIVE_BITS,SIX_BITS,SEVEN_BITS,EIGHT_BITS,NINE_BITS=7
}UART_BitData;

typedef enum
{
	Disabled,Even_Parity=2,Odd_Parity
}UART_Parity;

typedef enum
{
	one_bit,two_bit
}UART_StopBit;

typedef enum
{
	BR_4800 = 4800, BR_9600 = 9600, BR_14400 = 14400, BR_19200 = 19200, BR_38400 = 38400
}UART_BaudRate;

typedef struct{
	 UART_BitData bit_data;
	 UART_Parity parity;
	 UART_StopBit stop_bit;
	 UART_BaudRate baud_rate;
}UART_ConfigType;

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/*
 * Description :
 * Functional responsible for Initialize the UART device by:
 * 1. Setup the Frame format like number of data bits, parity bit type and number of stop bits.
 * 2. Enable the UART.
 * 3. Setup the UART baud rate.
 */
void UART_init(const UART_ConfigType * Config_Ptr);

/*
 * Description :
 * Functional responsible for send byte to another UART device.
 */
void UART_sendByte(const uint8 data);

/*
 * Description :
 * Functional responsible for receive byte from another UART device.
 */
uint8 UART_recieveByte(void);

/*
 * Description :
 * Send the required string through UART to the other UART device.
 */
void UART_sendString(const uint8 *Str);

/*
 * Description :
 * Receive the required string until the '#' symbol through UART from the other UART device.
 */
void UART_receiveString(uint8 *Str); // Receive until #

#endif /* UART_H_ */
