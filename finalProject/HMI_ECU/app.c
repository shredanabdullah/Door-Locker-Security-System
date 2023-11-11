/*
 ================================================================================================
 Name        : app.c
 Author      : shredan abdullah
 Date        : 05/11/2023
 ================================================================================================
 */

#include <util/delay.h>
#include "lcd.h"
#include "keypad.h"
#include "uart.h"
#include "timer.h"
#include "common_macros.h"
#include <avr/io.h> /* To use the SREG register */


#define WRONG_PASSWORD        0  /* Byte defines wrong data sent to control_MCU */
#define CORRECT_PASSWORD      1  /* Byte defines correct data sent to control_MCU */
#define REPEAT_BYTE           3  /* Byte defines wrong data sent to control_MCU and asks for repeating it */

uint8 g_matchingFlag = 0;
uint8 g_password[5];
uint8 g_passwordConfirm[5];
uint8 g_definedPasswordToBeSend[5];


Timer1_ConfigType s_timerConfigurations_15Sec = {0, 14649, clk_1024, CTC}; /*Timer configuration for 15 sec */
Timer1_ConfigType s_timerConfigurations_3Sec = {0, 2930, clk_1024, CTC};   /*Timer configuration for 3 sec */
Timer1_ConfigType s_timerConfigurations_60Sec = {0, 58594, clk_1024, CTC}; /*Timer configuration for 1 min */

/***************************
 *                             Functions Prototypes                            *
 ***************************/


void readPassword (void);
void enterPassword(void);
void mainDisplay (void);



void timerCallBack_15Sec (void);
void timerCallBack_3Sec (void);
void timerCallBack_60Sec (void);


int main(void)
{

	LCD_init (); /* Initialize LCD */
	/* UART configurations with 8 Bits data, No parity, one stop bit and 9600 baud rate*/

	UART_ConfigType s_configuration = {EIGHT_BITS, Disabled, one_bit, BR_9600};
	UART_init (&s_configuration);
	SET_BIT (SREG, 7);

	while(1)
	{
		//readPassword();

		switch (g_matchingFlag){
		     case WRONG_PASSWORD:  readPassword();              break;/*When there is no matching between new and confirmation passwords take new password again */
		     case CORRECT_PASSWORD: mainDisplay();  break;/* When the new pass and confirmation are matched start the system options*/
		}

//
	}
	 return 0;
}
void readPassword(void){
/* The Password */
		LCD_clearScreen();
		LCD_displayString ("Plz enter Pass:");
		LCD_moveCursor (1,0);

		enterPassword();

		/* The repeated password */
		LCD_clearScreen();
		LCD_displayString ("Plz re_enter the");
		LCD_moveCursor (1,0);
		LCD_displayString ("Same Pass:");
		LCD_moveCursor (1,10);

		enterPassword();
		g_matchingFlag = UART_recieveByte();
}
void enterPassword(void){
	uint8 i=0;
		for (;i<5;i++)
			{
				g_password[i] = KEYPAD_getPressedKey ();// 3mlt check 3la eldosa abl ma a3rd ngom 3shan lw enter m3rdsh ngma bardo

				if (g_password [i] == 13)/* ASCII of Enter */
				{
					break;
				}
				LCD_displayCharacter(g_password[i]);
				_delay_ms (450);
			}
			g_password[i] = '#';				/* For UART_recieveString function */
			g_password[i+1] = '\0';				/* For UART_sendString function */

			UART_sendString (g_password);/* Send the new password to control_ECU*/
     		_delay_ms (10);

}


void timerCallBack_15Sec (void)
{
	switch (g_matchingFlag)
	{
	case 'd':
		Timer1_init (&s_timerConfigurations_3Sec); /* Start to count 3 seconds for door to start locking again */
		Timer1_setCallBack(timerCallBack_3Sec);   /* Set the second call back */
		break;
	case 'e':
		LCD_clearScreen();
		g_matchingFlag = CORRECT_PASSWORD;             /* For system main options */
		Timer1_deInit ();						   /* De_initialize the timer */
	}
}


void timerCallBack_3Sec (void)
{
	/* Display door is locking after being unlocked for 3 seconds */
	LCD_clearScreen();
	LCD_moveCursor (0,4);
	LCD_displayString ("Door is");
	LCD_moveCursor (1,4);
	LCD_displayString ("Locking");
	g_matchingFlag = 'e';

	Timer1_init(&s_timerConfigurations_15Sec);  /* Start to count 15 seconds for door to be locked again */
	Timer1_setCallBack (timerCallBack_15Sec);	 /* Set the first call back */
}


void timerCallBack_60Sec (void)
{
	Timer1_deInit();                    /* De_initialize the timer */
	g_matchingFlag = CORRECT_PASSWORD;       /* For system main options */
}
void mainDisplay (void)
{
	/*they are both static 3shan kol mara andah eldisplay fe main loop marga3sh abda2 elinitialization bta3om tane enhom b zeros la ana 3aiza arga3 3la elkema eladema 3ade*/


	 static uint8 choice = 0;
	 static uint8 recieved = 0;/* Save the received confirmation byte from control_ECU */

	/* Print the main system options for first time and disable it for second and third password iterations */
if(recieved!=REPEAT_BYTE){
		LCD_clearScreen ();
		LCD_moveCursor (0,0);
		LCD_displayString ("+ : OPEN DOOR");
		LCD_moveCursor (1,0);
		LCD_displayString ("- : CHANGE PASS");

		choice = KEYPAD_getPressedKey ();       /* Take the user choice either open door or change pass */
		_delay_ms (300);
}
	switch (choice)
	{
	case '+':
		/* The Password */
		LCD_clearScreen();
		LCD_displayString ("Plz enter Pass:");
		LCD_moveCursor (1,0);

		enterPassword();

		recieved = UART_recieveByte();
		/* based on the received if repeatPassword is confirmed, open the door.If wrong after 3 iterations, open the buzzer.*/
		switch (recieved)
		{/*if two passwords are matched:
		• rotates motor for 15-seconds CW and display a message on the screen “Door is Unlocking”
		• hold the motor for 3-seconds.
		• rotates motor for 15-seconds A-CW and display a message on the screen “Door is Locking”*/
		case CORRECT_PASSWORD:
			UART_sendByte(choice);//to urt to control ECU
			LCD_clearScreen();
			LCD_moveCursor(0,5);
			LCD_displayString("Door is");
			LCD_moveCursor(1,4);
			LCD_displayString("Unlocking");

			Timer1_setCallBack (timerCallBack_15Sec);
			Timer1_init(&s_timerConfigurations_15Sec);

			g_matchingFlag = 'd';
			break;

		case WRONG_PASSWORD:
			Timer1_setCallBack (timerCallBack_60Sec);
			Timer1_init (&s_timerConfigurations_60Sec);
			LCD_clearScreen ();
			LCD_moveCursor (0,5);
			LCD_displayString ("error!");
		}
		break;

	case '-':
		/* The Password */
		LCD_clearScreen();
		LCD_displayString ("Plz enter Pass:");
		LCD_moveCursor (1,0);

		enterPassword();

		recieved = UART_recieveByte();

		/* based on the received if repeatPassword is confirmed, open the door.If wrong after 3 iterations, open the buzzer.*/
		switch (recieved)
		{
		case CORRECT_PASSWORD:
			UART_sendByte (choice);
			g_matchingFlag = WRONG_PASSWORD;  /* to start to take new password */
			break;

		case WRONG_PASSWORD:
			Timer1_setCallBack (timerCallBack_60Sec);
			Timer1_init (&s_timerConfigurations_60Sec);
			LCD_clearScreen ();
			LCD_moveCursor (0,5);
			LCD_displayString ("error!");
		}
	}
}


