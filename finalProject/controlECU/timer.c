/*
 * timer1.c
 *
 *  Created on: Oct 28, 2023
 *      Author: shredan abdullah
 */

#include "timer.h"
#include "std_types.h"
#include "common_macros.h"
#include <avr/io.h>
#include <avr/interrupt.h>

/*******************************************************************************
 *                                    Globals                                  *
 *******************************************************************************/
static volatile void (*g_callBack_Ptr)(void) = NULL_PTR;
/*******************************************************************************
 *                                    ISR                                      *
 *******************************************************************************/
//two ISR’s for Normal and Compare interrupts.

/* For calling the call back functions */
ISR (TIMER1_COMPA_vect)
{
	if (g_callBack_Ptr != NULL_PTR)
		{
			(*g_callBack_Ptr)();
		}
}

ISR (TIMER1_OVF_vect)
{
	if (g_callBack_Ptr != NULL_PTR)
	{
		(*g_callBack_Ptr)();
	}
}

/*● Description
⮚ Function to initialize the Timer driver*/
void Timer1_init(const Timer1_ConfigType * Config_Ptr){
	TCNT1=Config_Ptr->initial_value; /* Set the initial timer value */
	TCCR1B=(TCCR1B & 0xF8)|(Config_Ptr->prescaler);/*Clock Select(Prescaler Setup), Setup the first three bits in TCCR1B reg*/

	if(Config_Ptr->mode==NORMAL){
		TCCR1B=(TCCR1B & 0xF7)| ((Config_Ptr->mode)<<3);/*Waveform Generation Mode Bit, 0 for NORMAL Mode, clear the forth bit(WGM12) in TCCR1B reg*/
		SET_BIT(TIMSK,TIMER1_TIMSK_TOIE1);/*Overflow Interrupt Enable*/
	}
	else if(Config_Ptr->mode==CTC){
		 TCCR1B=(TCCR1B & 0xF7)| ((Config_Ptr->mode)<<3);/*Waveform Generation Mode Bit, 1 for CTC Mode, Set the forth bit(WGM12) in TCCR1B reg*/
		 OCR1A=Config_Ptr->compare_value; /* Set the required compare value */
		 SET_BIT(TIMSK,TIMER1_TIMSK_OCIE1A); /*Output Compare A Match Interrupt Enable*/
		 SET_BIT(TIMSK,TIMER1_TCCR1A_COM1A1);/*Compare Output Mode for Compare unit A, Clear OC1A on compare match (Set output to low level)*/
	}
}


/*● Description
⮚ Function to disable the Timer1.*/
void Timer1_deInit(void){
	TCCR1B=(TCCR1B & 0xF8); /*No_clock_source,disable the Timer1*/
}


/*● Description
⮚ Function to set the Call Back function address.*/
void Timer1_setCallBack(void(*a_ptr)(void)){
	g_callBack_Ptr = (volatile void*)a_ptr;
}


