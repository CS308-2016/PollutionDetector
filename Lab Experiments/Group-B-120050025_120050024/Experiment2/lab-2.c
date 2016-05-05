/*

* Author: Texas Instruments 

* Editted by: Saurav Shandilya, Vishwanathan Iyer 
	      ERTS Lab, CSE Department, IIT Bombay

* Description: This code will familiarize you with using GPIO on TMS4C123GXL microcontroller.

* Filename: lab-1.c

* Functions: setup(), ledPinConfig(), switchPinConfig(), main()

* Global Variables: none

*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"

// LOCK_F and CR_F - used for unlocking PORTF pin 0
#define LOCK_F (*((volatile unsigned long *)0x40025520))
#define CR_F   (*((volatile unsigned long *)0x40025524))



/*
 ------ Global Variable Declaration
*/

int sw2status = 0, sw1state =0 , sw2state = 0;
uint8_t ui8LED = 0,ui8PrevLED = 2;

/*

* Function Name: setup()

* Input: none

* Output: none

* Description: Set crystal frequency and enable GPIO Peripherals  

* Example Call: setup();

*/
void setup(void)
{
	SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
}

/*

* Function Name: ledPinConfig()

* Input: none

* Output: none

* Description: Set PORTF Pin 1, Pin 2, Pin 3 as output.

* Example Call: ledPinConfig();

*/

void ledPinConfig(void)
{
	//GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);  // Pin-1 of PORT F set as output. Modifiy this to use other 2 LEDs.

	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
}

/*

* Function Name: switchPinConfig()

* Input: none

* Output: none

* Description: Set PORTF Pin 0 and Pin 4 as input. Note that Pin 0 is locked.

* Example Call: switchPinConfig();

*/
void switchPinConfig(void)
{
	// Following two line removes the lock from SW2 interfaced on PORTF Pin0 -- leave this unchanged
	LOCK_F=0x4C4F434BU;
	CR_F=GPIO_PIN_0|GPIO_PIN_4;

	// GPIO PORTF Pin 0 and Pin4
	//GPIODirModeSet(GPIO_PORTF_BASE,GPIO_PIN_4,GPIO_DIR_MODE_IN); // Set Pin-4 of PORT F as Input. Modifiy this to use another switch
	GPIODirModeSet(GPIO_PORTF_BASE,GPIO_PIN_4|GPIO_PIN_0,GPIO_DIR_MODE_IN);
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0);
	GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_4|GPIO_PIN_0,GPIO_STRENGTH_12MA,GPIO_PIN_TYPE_STD_WPU);
}


int main(void)
{
	setup();
	ledPinConfig();
	switchPinConfig();

	uint32_t ui32Period;

	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);


	ui32Period = (SysCtlClockGet()/10 ) / 2;

	TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1);
	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();
	TimerEnable(TIMER0_BASE, TIMER_A);

	while(1)
		{
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, ui8LED);
		}

	/*---------------------

		* Write your code here
		* You can create additional functions

	---------------------*/
}



void Timer0IntHandler(void)
{
	// Clear the timer interrupt
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	// Read the current state of the GPIO pin and
	// write back the opposite state

	if (sw1state==0){
		if (!GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)){
			sw1state=1;
			ui8LED = 0;
		}
		else {
			ui8LED = 0;
			sw1state=0;
		}
	}
	else if (sw1state==1){
			if (!GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)){
				ui8LED = ui8PrevLED;
				sw1state=2;
				if(ui8PrevLED == 2)
					ui8PrevLED = 8;
				else if(ui8PrevLED == 8)
					ui8PrevLED = 4;
				else
					ui8PrevLED = 2;
			}
			else {
				ui8LED = 0;
				sw1state=0;
			}
	}
	else if (sw1state==2){
				if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)){
					ui8LED = 0;
					sw1state=0;
				}
				else {
					sw1state=2;
				}
	}

		if (sw2state==0){
			if (!GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)){
				sw2state=1;
			}
			else sw2state=0;
		}
		else if (sw2state==1){
				if (!GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)){
					sw2state=2;
					sw2status++;
				}
				else {
					sw2state=0;
				}
		}
		else if (sw2state==2){
					if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)){

						sw2state=0;
						GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
					}
					else sw2state=2;
		}


}
