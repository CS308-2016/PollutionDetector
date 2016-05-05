#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include"inc/hw_ints.h"
#include"driverlib/interrupt.h"
#include"driverlib/debug.h"
#include"driverlib/adc.h"

//declare variables
volatile uint32_t ui32AvgTemperature;
volatile uint32_t ui32TempinC;
volatile uint32_t ui32TempinF;

volatile uint32_t ui32SetTemp;

//clear screen
void clearscreen(){
	uint32_t i;
	for(i=0;i<100;i++)
			UARTCharPut(UART0_BASE,'\b');
	for(i=0;i<100;i++)
		UARTCharPut(UART0_BASE,' ');
	for(i=0;i<100;i++)
		UARTCharPut(UART0_BASE,'\b');
}
// Ask user to input temperature
uint32_t inputTemperature(){
	clearscreen();
	char string[25] = "Enter the temperature : ";
	uint32_t temperature=0,i;

	for(i=0;i<25;i++)
		UARTCharPut(UART0_BASE,string[i]);

	int flag=1;
	while (flag)
	{
		if (UARTCharsAvail(UART0_BASE)){
			char c = UARTCharGet(UART0_BASE);
			UARTCharPut(UART0_BASE,c);
			if(c>=48 && c<58){
				temperature = temperature*10 + (c-48);

			}else
				flag=0;
		}
	}

	clearscreen();
	char prompt[100]="Set Temperature updated to XX oC";
	prompt[28]=temperature%10+48;
	prompt[27]=(temperature/10)%10+48;
	prompt[30]=176;
	for(i=0;i<32;i++)
		UARTCharPut(UART0_BASE,prompt[i]);

	SysCtlDelay(SysCtlClockGet()); // Wait around 3 sec
	clearscreen();
	return temperature;
}

void UARTIntHandler(void)
{

	uint32_t ui32Status;
	char c;
	ui32Status = UARTIntStatus(UART0_BASE, true); //get interrupt status

	UARTIntClear(UART0_BASE, ui32Status); //clear the asserted interrupts

	while(UARTCharsAvail(UART0_BASE)) //loop while there are chars
	{
		c = UARTCharGet(UART0_BASE);
		if(c!='s')
			continue;

		ui32SetTemp = inputTemperature();
	}
}

//print temperature to terminal
void print_temp(uint32_t temperature){
	char string[100] = "Current temp = 25 oC , Set temp = 30 oC";
	string[18] = 176;
	string[16] = temperature%10+48;
	string[15] = (temperature/10)%10+48;

	string[37] = 176;
	string[35] = ui32SetTemp%10+48;
	string[34] = (ui32SetTemp/10)%10+48;

	int l = 40,i;
	clearscreen();
	for(i=0;i<l;i++)
		UARTCharPut(UART0_BASE,string[i]);

}



int main(void) {

	uint32_t ui32ADC0Value[4];

	ui32SetTemp = 25;

	// Set System CLock
	SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
	// Enaable UART
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	// Enable GPIO
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	// Enable ADC Peripheral
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

	//Allow the ADC12 to run at its default rate of 1Msps.
	ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);

	//our code will average all four samples of temperature sensor data	ta on sequencer 1 to calculate the temperature, so all four sequencer steps will measure the temperature sensor
	ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_TS);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_TS);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_TS);
	ADCSequenceStepConfigure(ADC0_BASE,1,3,ADC_CTL_TS|ADC_CTL_IE|ADC_CTL_END);

	//enable ADC sequencer 1.
	ADCSequenceEnable(ADC0_BASE, 1);

	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	IntMasterEnable();
	IntEnable(INT_UART0);
	UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_1|GPIO_PIN_3);

	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));



	while (1)
	{
		//indication that the ADC conversion process is complete
		ADCIntClear(ADC0_BASE, 1);
		//trigger the ADC conversion with software
		ADCProcessorTrigger(ADC0_BASE, 1);
		//wait for the conversion to complete
		while(!ADCIntStatus(ADC0_BASE, 1, false))
		{
		}
		//read the ADC value from the ADC Sample Sequencer 1 FIFO
		ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);
		//take upper bound average of reading
		ui32AvgTemperature = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3] + 2)/4;

		//convert to temperature in celsius/fahrenheit

		ui32TempinC = (1475 - ((2475 * ui32AvgTemperature)) / 4096)/10;
		ui32TempinF = ((ui32TempinC * 9) + 160) / 5;

		print_temp(ui32TempinC);
		SysCtlDelay(SysCtlClockGet() / 3); //delay ~1 sec
		//adjust leds according to set temperature value
		if(ui32TempinC < ui32SetTemp)
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 8);
		else
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 2);

	}
}
