#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/gpio.h"
#include"inc/hw_ints.h"
#include"driverlib/adc.h"
#include"driverlib/interrupt.h"
#include"driverlib/debug.h"

volatile uint32_t ui32TempAvg;
volatile uint32_t ui32TempValueC;
volatile uint32_t ui32TempValueF;
volatile uint32_t ui32TempSet;

void clearscreen(){
	uint32_t j;
	for(j=1;j<=100;j++)
		UARTCharPut(UART0_BASE,'\b');
	for(j=1;j<=100;j++)
		UARTCharPut(UART0_BASE,' ');
	for(j=1;j<=100;j++)
		UARTCharPut(UART0_BASE,'\b');
}

uint32_t inputtemp(){
	clearscreen();
	char str[25] = "Enter the temperature : ";
	uint32_t temp=0,j;

	for(j=0;j<25;j++)
		UARTCharPut(UART0_BASE,str[j]);

	int flag=1;
	while (flag)
	{
		if (UARTCharsAvail(UART0_BASE)){
			char c1 = UARTCharGet(UART0_BASE);
			UARTCharPut(UART0_BASE,c1);
			if(c1>=48 && c1<=57){
				temp = temp*10 + (c1-48);
			}
			else
				flag=0;
		}
	}

	clearscreen();
	char ack[100]="Set Temperature updated to XX oC";
	ack[28]=temp%10+48;
	ack[27]=(temp/10)%10+48;
	ack[30]=176;
	for(j=0;j<32;j++)
		UARTCharPut(UART0_BASE,ack[j]);

	SysCtlDelay(SysCtlClockGet());// waiting for ~3 seconds
	clearscreen();
	return temp;
}

void UARTIntHandler(void)
{

	uint32_t ui32Status;
	char c1;
	ui32Status = UARTIntStatus(UART0_BASE, true);

	UARTIntClear(UART0_BASE, ui32Status);

	while(UARTCharsAvail(UART0_BASE))  //keep iterating till there are chars
	{
		c1 = UARTCharGet(UART0_BASE);
		if(c1!='s')
			continue;

		ui32TempSet = inputtemp();
	}
}

int main(void) {

	uint32_t ui32ADC0Value[4];

	ui32TempSet = 25;
	// setting System CLock
	SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
	// enabling UART
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	//allowing the ADC12 to run at 1Msps (default rate)
	ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
	//averaging all four samples of temperature sensor data on sequencer 1 to get the temperature (all the 4 sequencer steps measure the temperatures sensor)
	ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_TS);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_TS);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_TS);
	ADCSequenceStepConfigure(ADC0_BASE,1,3,ADC_CTL_TS|ADC_CTL_IE|ADC_CTL_END);
	//enabling ADC sequencer as 1
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
		//telling that the ADC conversion is done
		ADCIntClear(ADC0_BASE, 1);
		//triggering the ADC conversion
		ADCProcessorTrigger(ADC0_BASE, 1);
		while(!ADCIntStatus(ADC0_BASE, 1, false))
		{
		}
		//reading  the ADC value from the ADC Sample Sequencer 1
		ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);
		ui32TempAvg = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3] + 2)/4;
		ui32TempValueC = (1475 - ((2475 * ui32TempAvg)) / 4096)/10;
		ui32TempValueF = ((ui32TempValueC * 9) + 160) / 5;

		//print
		char s[100] = "Current Temp = 25 oC , Set Temp = 30 oC";
		s[18] = 176;
		s[16] = ui32TempValueC%10+48;
		s[15] = (ui32TempValueC/10)%10+48;

		s[37] = 176;
		s[35] = ui32TempSet%10+48;
		s[34] = (ui32TempSet/10)%10+48;

		int j,len = 40;
		clearscreen();
		for(j=0;j<len;j++)
			UARTCharPut(UART0_BASE,s[j]);

		SysCtlDelay(SysCtlClockGet() / 3);
		if(ui32TempValueC < ui32TempSet)
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 8);
		else
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 2);

	}
}
