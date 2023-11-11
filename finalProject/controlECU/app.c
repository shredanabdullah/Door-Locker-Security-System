/*
 * app.c
 * CONTROL_ECU
 *  Created on: Oct 29, 2023
 *      Author: shredan abdullah
 */


/*
 * CONTROL_ECU is responsible for all the processing and decisions in the system like password
checking, open the door and activate the system alarm.*/
#include "std_types.h"
#include "dc_motor.h"
#include "twi.h"
#include <avr/io.h>
#include <util/delay.h>
#include "buzzer.h"
#include "external_eeprom.h"
#include "uart.h"
#include "timer.h"
#include "common_macros.h"


#define WRONG_PASSWORD        0  /* Byte defines wrong data sent to control_MCU */
#define CORRECT_PASSWORD      1  /* Byte defines correct data sent to control_MCU */
#define REPEAT_BYTE           3  /* Byte defines wrong data sent to control_MCU and asks for repeating it */
#define EEPROM_ADDRESS         0   /* First Address in EEPROM Password */

/*defines if matching between passwords occurs or not */
uint8 g_matchingFlag = 0;
uint8 g_passwordControlEcu[7];
uint8 g_passwordConfirmControlEcu[7];
uint8 g_definedPassword[7];

/*Timer configuration for 15 sec */
Timer1_ConfigType s_timerConfigurations_15Sec = {13885, 0, clk_1024, NORMAL};
/*Timer configuration for 3 sec */
Timer1_ConfigType s_timerConfigurations_3Sec = {42099, 0, clk_1024, NORMAL};
/*Timer configuration for 1 min */
Timer1_ConfigType s_timerConfigurations_60Sec = {55538, 0, clk_1024, NORMAL};


/*******************************************************************************
 *                             Functions Prototypes                            *
 *******************************************************************************/

/*
 * Description:
 * 1. Receive the new password and its confirmation from HMI_ECU.
 * 2. Compare the 2 passwords.
 * 3. If matched, send the confirm byte to HMI_ECU and save the password in EEPROM.
 * 4. If not matched, send the wrong byte to HMI_ECU and repeat again.
 */
void receivePassword (void);

/*
 * Description:
 * 1. Receive the user input password for selecting either open door or change pass from HMI_ECU.
 * 2. Compare it with the password saved in EEPROM.
 * 3. If matched, receive the user choice byte and if '+' rotate the motor, if '-' change password.
 * 4. If not matched, send repeat byte to HMI_ECU to ask for password 2 more times.
 * 5. If matched in the 2 next iterations take the action.
 * 6. If not matched in the 3 iterations, send wrong byte to HMI_ECU and start the buzzer.
 */
void getDefinedPassword (void);

/*
 * Description:
 * Timer1 first call back function after counting 15 seconds:
 * 1. First call stops the motor for 3 seconds after 15 seconds and initialize the timer for counting 3 seconds.
 * 2. Second call stops the motor after gate is closed and de-initialise the timer.
 */
void timerCallBack_15Sec (void);

/*
 * Description:
 * Timer1 second call back function after counting 3 seconds:
 * 1. After being called initialize the timer for counting another 15 seconds for the door to start locking.
 */
void timerCallBack_3Sec (void);

/*
 * Description:
 * Timer1 third call back function after counting 1 minute:
 * 1. After being called stops the buzzer ringing which started when 3 consecutive passwords are wrong.
 */
void timerCallBack_60Sec (void);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(void){

	/*initialization*/
	    Buzzer_init();	/* Initialize buzzer */
	    DcMotor_Init(); /* Initialize DC_motor */
		/* I2C configurations with address of 1 and 400 Kbit/sec (Fast Mode)*/
	    TWI_ConfigType s_twiConfiguration = {1, 400};
		TWI_init (&s_twiConfiguration);
		/* UART configurations with 8 Bits data, No parity, one stop bit and 9600 baud rate*/
		UART_ConfigType s_uartConfiguration = {EIGHT_BITS, Disabled, one_bit, BR_9600};
		UART_init (&s_uartConfiguration);


		SET_BIT (SREG, 7);/* Enable I-bit */

	/*super loop*/
	 	while(1){
	 		switch (g_matchingFlag){
	 		 case 0:receivePassword(); break;/* When there is no matching between new and confirmation passwords receive new password again */
	 		 case 1:getDefinedPassword();  break;/* When the new pass and confirmation are matched start the system options by receiving the user choice*/
	 		}
	 	}
	 return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Description:
 * Timer1 first call back function after counting 15 seconds:
 * 1. First call stops the motor for 3 seconds after 15 seconds and initialize the timer for counting 3 seconds.
 * 2. Second call stops the motor after gate is closed and de-initialise the timer.
 */
void timerCallBack_15Sec (void)
{
	static uint8 counter = 0;
	counter++;
	switch (counter)
	{
	case 2:
		DcMotor_Rotate(stop, 0);					   /* Stop the motor after being unlocking for 15 seconds */
		Timer1_deInit ();
		Timer1_setCallBack (timerCallBack_3Sec);   /* Set the second call back */
		Timer1_init (&s_timerConfigurations_3Sec); /* Start to count 3 seconds for door to start locking again */
		break;
	case 4:
		DcMotor_Rotate(stop, 0);						   /* Stop the motor after being locked again */
		Timer1_deInit ();						   /* De_initialize the timer */
		counter = 0;
	}
}

/*
 * Description:
 * Timer1 second call back function after counting 3 seconds:
 * 1. After being called initialize the timer for counting another 15 seconds for the door to start locking.
 */
void timerCallBack_3Sec (void)
{
	DcMotor_Rotate(A_CW, 100);					/* rotate motor CCW after being stopped for 3 seconds */
	Timer1_deInit ();
	Timer1_init (&s_timerConfigurations_15Sec); /* Start to count 15 seconds for door to be locked again */
	Timer1_setCallBack (timerCallBack_15Sec);   /* Set the first call back */
}

/*
 * Description:
 * Timer1 third call back function after counting 1 minute:
 * 1. After being called stops the buzzer ringing which started when 3 consecutive passwords are wrong.
 */
void timerCallBack_60Sec (void)
{
	static uint8 counter = 0;
	counter++;
	/* Because it takes 8 ISR to count 1 minute */
	if (counter == 8)
	{
		Buzzer_off ();							/* Stop the buzzer after 1 minute */
		Timer1_deInit ();						/* De_initialize the timer */
		counter = 0;
	}
}


void receivePassword(){
	uint8 counter=0;
	uint8 i;
	/* Receive the 2 passwords */
		UART_receiveString (g_passwordControlEcu);
		UART_receiveString (g_passwordConfirmControlEcu);
		for(i=0;i<5;i++){
			if(g_passwordControlEcu[i]!=g_passwordConfirmControlEcu[i]){
				//g_matchingFlag=0;
				UART_sendByte (WRONG_PASSWORD);
			}
			else counter++;
		}
		if(counter==5){
			UART_sendByte(CORRECT_PASSWORD);                                         /* Send confirm byte */
			g_matchingFlag = 1;
			for( i = 0; i < 5; i++){
			     EEPROM_writeByte ((uint16)(EEPROM_ADDRESS + i), g_passwordControlEcu[i]);  /* Save password in EEPROM */
				 _delay_ms (10);
			}
		}
}

/*
 * Description:
 * 1. Receive the user input password for selecting either open door or change pass from HMI_ECU.
 * 2. Compare it with the password saved in EEPROM.
 * 3. If matched, receive the user choice byte and if '+' rotate the motor, if '-' change password.
 * 4. If not matched, send repeat byte to HMI_ECU to ask for password 2 more times.
 * 5. If matched in the 2 next iterations take the action.
 * 6. If not matched in the 3 iterations, send wrong byte to HMI_ECU and start the buzzer.
 */
void getDefinedPassword (void)
{
	uint8 i = 0;
	uint8 counter = 0;
	uint8 recieved = 0;
	static uint8 wrongIterations = 0; /* For counting the wrong pass */

	UART_receiveString (g_definedPassword);								  /* Receive the user input pass */

	/* Receive the pass stored in EEPROM */
	for (i = 0; i < 5; i++)
	{
		EEPROM_readByte ((uint16)(EEPROM_ADDRESS + i), g_passwordConfirmControlEcu + i);
		_delay_ms (10);
	}
	g_passwordConfirmControlEcu[i] = '\0';

	/* Start comparing the 2 passwords */
	i = 0;
	while ((g_definedPassword[i] != '\0') && (g_passwordConfirmControlEcu[i] != '\0'))
	{
		if (g_definedPassword[i] != g_passwordConfirmControlEcu[i])
		{
			break;
		}
		i++;
		counter++;
	}
	if (counter == 5)
	{
		UART_sendByte (CORRECT_PASSWORD);/* Send confirm byte */
		recieved = UART_recieveByte ();/* Receive the user choice */
		wrongIterations = 0; /*3shan a-count 3la nedef lma y7sal moshkela(wrong iteration count) b3d kda*/
		if (recieved == '+')/* If open the door */
		{
			DcMotor_Rotate(CW, 150);/* Start rotating the motor CW */
			Timer1_setCallBack (timerCallBack_15Sec);
			Timer1_init (&s_timerConfigurations_15Sec);/* Rotate for 15 seconds */
		}
		else if (recieved == '-') /* If change pass */
		{
			g_matchingFlag = 0;/* to call recieveCheckNewPassword */
		}
	}
	else
	{
		wrongIterations++;													  /* Increment wrong iterations */
		if (wrongIterations == 3)	/*(third consecutive time)*/
		{
			UART_sendByte(WRONG_PASSWORD);										  /* Send wrong byte */
			Buzzer_on();													  /* Start the buzzer */
			Timer1_setCallBack (timerCallBack_60Sec);
			Timer1_init (&s_timerConfigurations_60Sec);						  /* Start counting 60 seconds */
			wrongIterations = 0;											  /* Restart the wrong iterations again */
		}
		else
		{
			UART_sendByte(REPEAT_BYTE);									  /* If less than 3 send repeat */
		}
	}
}
