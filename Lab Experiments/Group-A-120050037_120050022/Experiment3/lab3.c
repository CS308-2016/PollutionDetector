#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"


#define PWM_FREQUENCY 55

int main(void)
{
	volatile uint32_t ui32Load;
	volatile uint32_t ui32PWMClock;
	volatile int ui8Adjustr,ui8Adjustg,ui8Adjustb;
	ui8Adjustr = 254;
	ui8Adjustg = 85;
	ui8Adjustb = 85;

	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
	SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
	//SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM2);
	//SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM3);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	//GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);
	GPIOPinConfigure(GPIO_PF1_M1PWM5);
	GPIOPinConfigure(GPIO_PF2_M1PWM6);
	GPIOPinConfigure(GPIO_PF3_M1PWM7);

	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
	GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_DIR_MODE_IN);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	ui32PWMClock = SysCtlClockGet() / 64;
	ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;
	PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN);
	PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, ui32Load);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, ui32Load);

	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8Adjustr * ui32Load / 1000);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8Adjustg * ui32Load / 1000);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8Adjustb * ui32Load / 1000);
	PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT|PWM_OUT_6_BIT|PWM_OUT_7_BIT, true);
	PWMGenEnable(PWM1_BASE, PWM_GEN_2);
	PWMGenEnable(PWM1_BASE, PWM_GEN_3);

	///////////////

	int mode = 0;
	int man_mode = 1;
	int incr = 1, incg = 1, incb = -1;
	int wait = 100000;

	int sw2_pressed = 0, sw1_pressed = 0;

	//////////////

	while(1)
	{
		if (mode == 0) {					// Mode 0
			ui8Adjustr += incr;				// Whether intensity of RED   light should be increased or decreased
			ui8Adjustg += incg;				// Whether intensity of GREEN light should be increased or decreased
			ui8Adjustb += incb;				// Whether intensity of BLUE  light should be increased or decreased
			if ((ui8Adjustr < 10) || (ui8Adjustr > 254)) incr = incr * -1;		// Checking boundary conditions
			if ((ui8Adjustg < 10) || (ui8Adjustg > 254)) incg = incg * -1;
			if ((ui8Adjustb < 10) || (ui8Adjustb > 254)) incb = incb * -1;
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8Adjustr * ui32Load / 1000);	// Setting the intensity
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8Adjustg * ui32Load / 1000);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8Adjustb * ui32Load / 1000);
			// Checking Buttons pressed and setting mode to 1
			if ((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00) && (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00)) {
				mode = 1;
				wait = 100000;
				ui8Adjustr = 10;
				ui8Adjustg = 10;
				ui8Adjustb = 10;
				SysCtlDelay(1000000);
			}
			// Checking Button pressed and making transitions faster
			if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00) {
				wait = wait - 20000;
				if (wait < 20000) wait = 20000;
			}
			// Making transitions slower
			else if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00) {
				wait = wait + 20000;
				if (wait > 200000) wait = 200000;
			}
			SysCtlDelay(wait);
		}
		else {							// Mode is not 0
			if (sw2_pressed >= 2) {		// Checking if given conditions are met
				if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)!=0x00) {
					if (sw1_pressed > 0) {
						man_mode = sw1_pressed-1;
						if (man_mode > 2) man_mode = 2;
						ui8Adjustr = 254;
					}
					sw2_pressed = 0;
					sw1_pressed = 0;
				}
				if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00) {
					sw1_pressed++;
				}
			}
			if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00) {
				sw2_pressed++;
			}
			if (man_mode == 0) {	// Manual mode 0
				if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00) {
					ui8Adjustr+=10;	// Increasing RED led intensity
					if (ui8Adjustr > 254) ui8Adjustr = 254;
				}
				else if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00) {
					ui8Adjustr-=10;	// Decreasing RED led intensity
					if (ui8Adjustr < 10) ui8Adjustr = 10;
				}
				ui8Adjustg = 10;
				ui8Adjustb = 10;
			}
			else if (man_mode == 1) {	// Manual mode 1
				if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00) {
					ui8Adjustg+=10;	// Increasing GREEN led intensity
					if (ui8Adjustg > 254) ui8Adjustg = 254;
				}
				else if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00) {
					ui8Adjustg-=10;	// Decreasing GREEN led intensity
					if (ui8Adjustg < 10) ui8Adjustg = 10;
				}
				ui8Adjustr = 10;
				ui8Adjustb = 10;
			}
			else if (man_mode == 2) {	// Manual mode 2
				if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00) {
					ui8Adjustb+=10;	// Increasing BLUE led intensity
					if (ui8Adjustb > 254) ui8Adjustb = 254;
				}
				else if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00) {
					ui8Adjustb-=10;	// Decreasing BLUE led intensity
					if (ui8Adjustb < 10) ui8Adjustb = 10;
				}
				ui8Adjustg = 10;
				ui8Adjustr = 10;
			}
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8Adjustr * ui32Load / 1000);		// Setting the intensity as per above conditions
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8Adjustb * ui32Load / 1000);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8Adjustg * ui32Load / 1000);

			SysCtlDelay(1000000);
		}
	}

}

