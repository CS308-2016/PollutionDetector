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
#include "driverlib/debug.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"

// LOCK_F and CR_F - used for unlocking PORTF pin 0
#define LOCK_F (*((volatile unsigned long *)0x40025520))
#define CR_F   (*((volatile unsigned long *)0x40025524))


#define PWM_FREQUENCY 55


/*
 ------ Global Variable Declaration
*/
bool automode = true;
uint8_t switchtype=0;//1 = sw1, 2 = sw2, 3 = sw1sw2, 4 = sw1sw2sw2, 5 = sw1sw2long, 0 = none
uint8_t manmode=1;//manmode number
int ui8Adjust1,ui8Adjust2,ui8Adjust3;


void checkswitch(){
	uint32_t iters,iters1;
	if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00 && GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00){//both sw
		iters = 0;
		while(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00 && GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00){
			SysCtlDelay(100000);
			iters ++;
		}
		if(iters > 10){
			manmode = 3;
			switchtype = 5;
			return;
		}
		switchtype = 0;
		return ;
	} else if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00){//sw1
		iters = 0;
		while(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00){
			if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00){//sw2
				iters1 = 0;
				while(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00 && GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00){
					SysCtlDelay(100000);
					iters1 ++;
				}
				if(iters1 > 10){
					manmode = 3;
					switchtype = 5;
					return ;
				}
				switchtype = 0;
				return ;
			}
			SysCtlDelay(100000);
			iters ++;
		}
		switchtype = 1;
		return ;
	} else if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00){//sw2
		iters = 0;
		switchtype = 2;
		while(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00){
			if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00){
				iters1 = 0;
				while(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00 && GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00){
					SysCtlDelay(100000);
					iters1 ++;
				}
				switchtype ++;
			}

			SysCtlDelay(100000);
			iters ++;
		}
		if(switchtype == 3) manmode = 1;
		else if(switchtype == 4) manmode = 2;
		if(switchtype < 5) return ;
		switchtype = 4; manmode = 2;
		return ;
	} else switchtype = 0;
}


int main(void)
{
	volatile uint32_t ui32Load;
	volatile uint32_t ui32PWMClock;

	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
	SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);//The Tiva Launchpad has two modules (0 and 1). Module 1 covers the LED pins
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

//	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);
//	GPIOPinConfigure(GPIO_PF1_M1PWM5);

	//Configure PF1,PF2,PF3 Pins as PWM
	GPIOPinConfigure(GPIO_PF1_M1PWM5);
	GPIOPinConfigure(GPIO_PF2_M1PWM6);
	GPIOPinConfigure(GPIO_PF3_M1PWM7);
	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);




	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
	GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_DIR_MODE_IN);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	ui32PWMClock = SysCtlClockGet() / 64;
	ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;
//PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN);
    //Configure PWM Options
    //PWM_GEN_2 Covers M1PWM4 and M1PWM5
    //PWM_GEN_3 Covers M1PWM6 and M1PWM7
    PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

//PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, ui32Load);
    //Set the Period (expressed in clock ticks)
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, ui32Load);
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, ui32Load);



//PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8Adjust * ui32Load / 1000);



//PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, true);
	PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT | PWM_OUT_6_BIT | PWM_OUT_7_BIT, true);
//PWMGenEnable(PWM1_BASE, PWM_GEN_2);
    // Enable the PWM generator
    PWMGenEnable(PWM1_BASE, PWM_GEN_2);
    PWMGenEnable(PWM1_BASE, PWM_GEN_3);



    bool automode=true;
    uint32_t delay=100000;// for auto mode speed


    ui8Adjust1 = 255, ui8Adjust2 = 0, ui8Adjust3 = 0;
    //Set PWM duty
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5,ui8Adjust1 * ui32Load / 1000);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,ui8Adjust2 * ui32Load / 1000);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,ui8Adjust3 * ui32Load / 1000);
    uint8_t inc1, inc2, inc3;
    inc1 = inc3 = 1, inc2 = -1;
	while(1)
	{
		if(automode){
			checkswitch();
			if(switchtype == 1){//sw1 increases speed
				delay /= 10;
				if(delay == 0) delay = 10000;
				switchtype = 0;

			}
			else if(switchtype == 2){//sw2 decreases
				delay *= 10;
				if(delay == 100000000) delay = 1000000;
				switchtype = 0;
			}
			else if(switchtype != 0){
				automode = false;
				continue;
			}

			if(ui8Adjust1 && ui8Adjust2) ui8Adjust1 --, ui8Adjust2++;
			else if(ui8Adjust2 && ui8Adjust3) ui8Adjust2 --, ui8Adjust3++;
			else if(ui8Adjust3 && ui8Adjust1) ui8Adjust3 --, ui8Adjust1++;
			else if(ui8Adjust1) ui8Adjust1 --, ui8Adjust2++;
			else if(ui8Adjust2) ui8Adjust2 --, ui8Adjust3++;
			else if(ui8Adjust3) ui8Adjust3 --, ui8Adjust1++;



		    //Set PWM duty
		    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5,ui8Adjust1 * ui32Load / 1000);
		    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,ui8Adjust2 * ui32Load / 1000);
		    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,ui8Adjust3 * ui32Load / 1000);
			SysCtlDelay(delay);
		}
		else {
			checkswitch();
			if(manmode == 1){
				if(switchtype == 1) ui8Adjust1+=20;
				else if(switchtype == 2) ui8Adjust1-=20;
			}
			else if(manmode == 2){
				if(switchtype == 1) ui8Adjust2+=20;
				else if(switchtype == 2) ui8Adjust2-=20;
			}
			else if(manmode == 3){
				if(switchtype == 1) ui8Adjust3+=20;
				else if(switchtype == 2) ui8Adjust3-=20;
			}

			if(ui8Adjust1 > 255) ui8Adjust1 = 255;
			if(ui8Adjust2 > 255) ui8Adjust2 = 255;
			if(ui8Adjust3 > 255) ui8Adjust3 = 255;

			if(ui8Adjust1 < 0) ui8Adjust1 = 0;
			if(ui8Adjust2 < 0) ui8Adjust2 = 0;
			if(ui8Adjust3 < 0) ui8Adjust3 = 0;

		    //Set PWM duty
		    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5,ui8Adjust1 * ui32Load / 1000);
		    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,ui8Adjust2 * ui32Load / 1000);
		    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,ui8Adjust3 * ui32Load / 1000);
			SysCtlDelay(100000);

		}
	}
}


