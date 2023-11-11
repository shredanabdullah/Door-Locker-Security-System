/*
 * buzzer.c
 *
 *  Created on: Oct 29, 2023
 *      Author: shredan abdullah
 */
#include "buzzer.h"
#include "gpio.h"

/*
● Description
⮚ Setup the direction for the buzzer pin as output pin through the GPIO driver.
⮚ Turn off the buzzer through the GPIO.
*/
void Buzzer_init(void){
	GPIO_setupPinDirection( BUZZER_PORT,  BUZZER_PIN,  PIN_OUTPUT);
	GPIO_writePin( BUZZER_PORT, BUZZER_PIN , LOGIC_LOW);
}
/*
● Description
⮚ Function to enable the Buzzer through the GPIO
*/
void Buzzer_on(void){
	GPIO_writePin( BUZZER_PORT, BUZZER_PIN , LOGIC_HIGH);
}
/*
● Description
⮚ Function to disable the Buzzer through the GPIO.
*/
void Buzzer_off(void){
	GPIO_writePin( BUZZER_PORT, BUZZER_PIN , LOGIC_LOW);
}
