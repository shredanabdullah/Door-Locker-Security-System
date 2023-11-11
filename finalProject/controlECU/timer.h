/*
 * timer1.h
 *
 *  Created on: Oct 28, 2023
 *      Author: shredan abdullah
 */
/*
 Implement a full Timer driver for TIMER1 with the configuration technique.
 The Timer1 Driver should be designed using the Interrupts with the callback’s technique.
 The Timer1 Driver should support both normal and compare modes and it should be
 configured through the configuration structure passed to the init function.
 The Timer Driver has 3 functions and two ISR’s for Normal and Compare interrupts.
 */
#ifndef TIMER_H_
#define TIMER_H_

#include "std_types.h"

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/

#define TIMER1_PORT           4
#define TIMER1_PIN            8

#define TIMER1_TIMSK_TOIE1    2
#define TIMER1_TIMSK_OCIE1A   4
#define TIMER1_TCCR1A_COM1A1   7


/*******************************************************************************
 *                               Types Declaration                             *
 *******************************************************************************/
typedef enum
{
	No_clock_source,clk_1,clk_8,clk_64,clk_256,clk_1024
}Timer1_Prescaler;

typedef enum
{
	NORMAL, CTC /*form Waveform Generation Mode Bit Description*/
}Timer1_Mode;


typedef struct {
	 uint16 initial_value;
	 uint16 compare_value; // it will be used in compare mode only.
	 Timer1_Prescaler prescaler;
	 Timer1_Mode mode;
} Timer1_ConfigType;


/*******************************************************************************
 *                              Functions Prototypes                           *
 *******************************************************************************/

/*● Description
⮚ Function to initialize the Timer driver*/
void Timer1_init(const Timer1_ConfigType * Config_Ptr);


/*● Description
⮚ Function to disable the Timer1.*/
void Timer1_deInit(void);


/*● Description
⮚ Function to set the Call Back function address.*/
void Timer1_setCallBack(void(*a_ptr)(void));


#endif /* TIMER_H_ */
