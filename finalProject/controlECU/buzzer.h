/*
 * buzzer.h
 *
 *  Created on: Oct 29, 2023
 *      Author: shredan abdullah
 */

#ifndef BUZZER_H_
#define BUZZER_H_

#include "std_types.h"

/*******************************************************************************
 *                             static configurations                                  *
 *******************************************************************************/

#define BUZZER_PORT           3
#define BUZZER_PIN            6


/*******************************************************************************
 *                              Functions Prototypes                           *
 *******************************************************************************/
/*
● Description
⮚ Setup the direction for the buzzer pin as output pin through the
GPIO driver.
⮚ Turn off the buzzer through the GPIO.
*/
void Buzzer_init(void);
/*
● Description
⮚ Function to enable the Buzzer through the GPIO
*/
void Buzzer_on(void);
/*
● Description
⮚ Function to disable the Buzzer through the GPIO.
*/
void Buzzer_off(void);
#endif /* BUZZER_H_ */
