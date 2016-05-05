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
{ //initialize variables corresponding to PWM and intensities
	volatile uint32_t ui32Load;
	volatile uint32_t ui32PWMClock;
	volatile int ui8redi,ui8greeni,ui8bluei;
	ui8redi = 254;
	ui8greeni = 85;
	ui8bluei = 85;

	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
	SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

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

	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8redi * ui32Load / 1000);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8greeni * ui32Load / 1000);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8bluei * ui32Load / 1000);
	PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT|PWM_OUT_6_BIT|PWM_OUT_7_BIT, true);
	PWMGenEnable(PWM1_BASE, PWM_GEN_2);
	PWMGenEnable(PWM1_BASE, PWM_GEN_3);

	///////////////
//initialize variables corresponding to operation modes,wait time and  switches
	int automatic_mode = 0;
	int manual_mode = 1;
	int red_inc = 1, green_inc = 1, blue_inc = -1;
	int delay = 100000;

	int sw2down = 0, sw1down = 0;

	//////////////

	while(1)
	{
		//automatic mode operation
		if (automatic_mode == 0) {
			ui8redi += red_inc;
			ui8greeni += green_inc;
			ui8bluei += blue_inc;
			if ((ui8redi < 10) || (ui8redi > 254)) red_inc = red_inc * -1;
			if ((ui8greeni < 10) || (ui8greeni > 254)) green_inc = green_inc * -1;
			if ((ui8bluei < 10) || (ui8bluei > 254)) blue_inc = blue_inc * -1;
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8redi * ui32Load / 1000);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8greeni * ui32Load / 1000);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8bluei * ui32Load / 1000);
			if ((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00) && (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00)) {
				automatic_mode = 1;
				delay = 100000;
				ui8redi = 10;
				ui8greeni = 10;
				ui8bluei = 10;
				SysCtlDelay(1000000);
			}
			if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00) {
				delay = delay - 20000;
				if (delay < 20000) delay = 20000;
			}
			else if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00) {
				delay = delay + 20000;
				if (delay > 200000) delay = 200000;
			}
			SysCtlDelay(delay);
		}
		else {
			if (sw2down >= 2) {
				if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)!=0x00) {
					if (sw1down > 0) {
						manual_mode = sw1down-1;
						if (manual_mode > 2) manual_mode = 2;
						ui8redi = 254;
					}
					sw2down = 0;
					sw1down = 0;
				}
				if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00) {
					sw1down++;
				}
			}
			if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00) {
				sw2down++;
			}
			//manual mode operation
			if (manual_mode == 0) {
				if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00) {
					ui8redi+=10;
					if (ui8redi > 254) ui8redi = 254;
				}
				else if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00) {
					ui8redi-=10;
					if (ui8redi < 10) ui8redi = 10;
				}
				ui8greeni = 10;
				ui8bluei = 10;
			}
			else if (manual_mode == 1) {
				if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00) {
					ui8greeni+=10;
					if (ui8greeni > 254) ui8greeni = 254;
				}
				else if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00) {
					ui8greeni-=10;
					if (ui8greeni < 10) ui8greeni = 10;
				}
				ui8redi = 10;
				ui8bluei = 10;
			}
			else if (manual_mode == 2) {
				if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00) {
					ui8bluei+=10;
					if (ui8bluei > 254) ui8bluei = 254;
				}
				else if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00) {
					ui8bluei-=10;
					if (ui8bluei < 10) ui8bluei = 10;
				}
				ui8greeni = 10;
				ui8redi = 10;
			}
			//Pulse Width Modulation to generate intermediate colors
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8redi * ui32Load / 1000);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8bluei * ui32Load / 1000);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8greeni * ui32Load / 1000);

			SysCtlDelay(1000000);
		}

	}

}

