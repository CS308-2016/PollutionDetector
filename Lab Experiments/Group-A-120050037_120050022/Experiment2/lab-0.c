#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#define LOCK_F (*((volatile unsigned long *)0x40025520))
#define CR_F   (*((volatile unsigned long *)0x40025524))

//initializing the variables for state, led color and counter
int state=0,led=0,prev_led=8,counter=0,state2=0;

void switchPinConfig(void)
{
	// Following two line removes the lock from SW2 interfaced on PORTF Pin0 -- leave this unchanged
	LOCK_F=0x4C4F434BU;
	CR_F=GPIO_PIN_0|GPIO_PIN_4;
	// GPIO PORTF Pin 0 and Pin4
	GPIODirModeSet(GPIO_PORTF_BASE,GPIO_PIN_4,GPIO_DIR_MODE_IN); // Set Pin-4 of PORT F as Input. Modifiy this to use another switch
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
	GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_4,GPIO_STRENGTH_12MA,GPIO_PIN_TYPE_STD_WPU);
	GPIODirModeSet(GPIO_PORTF_BASE,GPIO_PIN_0,GPIO_DIR_MODE_IN); // Set Pin-4 of PORT F as Input. Modifiy this to use another switch
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0);
	GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_0,GPIO_STRENGTH_12MA,GPIO_PIN_TYPE_STD_WPU);
}

int main(void)
{
	uint32_t ui32Period;
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	switchPinConfig();
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	ui32Period = (SysCtlClockGet() / 10) / 2;
	TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1);
	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();
	TimerEnable(TIMER0_BASE, TIMER_A);
	while(1)
	{
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, led);
	}
}
void Timer0IntHandler(void)
{
	// Clear the timer interrupt
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	// Read the current state of the GPIO pin and
	// write back the opposite state

	//for led
	if (state==0){ // if in idle state
		if (!GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)){
			state=1; // goes to press state
			led = prev_led*2; // change the led color
			if (led == 16) led = 2;

		}
		else state=0; // remains in idle state
	}
	else if (state==1){ // if in press state 
			if (!GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)){
				state=2; // goes to release state
			}
			else {
				prev_led=led;
				led=0;
				state=0; // goes to idle state
			}
	}
	else if (state==2){ // if in release state
				if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)){
					prev_led=led;
					led=0;
					state=0; // goes to idle state
				}
				else state=2; // remaines in release state
	}

	//for counter
		if (state2==0){ // if in idle state
			if (!GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)){
				counter++; // increment the counter
				state2=1; // goes to press state
			}
			else state2=0; // remaines in idle state
		}
		else if (state2==1){ // if in press state
				if (!GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)){
					state2=2; // goes to release state
				}
				else {
					state2=0; // goes to idle state
				}
		}
		else if (state2==2){ // if in release state
					if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)){
						state2=0; // goes to idle state
					}
					else state2=2; // remaines in release state
		}


}
