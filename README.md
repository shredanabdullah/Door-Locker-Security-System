# Door-Locker-Security-System
C Project - Based on Atmega32 Microcontroller

Developed a system that takes a password & confirms it using keypad, password is hidden into (*) on an LCD , previous features are implemented in HMI_ECU, Password is sent to control_ECU using UART to be stored in external EEPROM to compare the input with it for future trials of opening the door, a buzzer is set in case of 3 false trials
Drivers: GPIO, Keypad, LCD, Timer, UART, I2C, EEPROM, Buzzer and DC-Motor
