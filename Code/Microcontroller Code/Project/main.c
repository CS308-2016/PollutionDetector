/*
 * main.h
 *
 *  Created on: Aug 14, 2015
 *      Author: secakmak
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/fpu.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/timer.h"
#include "driverlib/adc.h"

#include "utils/uartstdio.h"
#include "utils/cmdline.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"

#include "DelayTimer.h"

static char COMMAND[128];
static char ReceivedData[512];
static char str[128];

char mystr[128];

bool process = true;
bool ProcessRoutine();

char *Substring(char *src, char *dst, int start, int stop);
int SearchIndexOf(char src[], char str[]);
char* itoa(int i, char b[]);
void ftoa(float f,char *buf);

#define Period  320000 //(16000000/50) 50Hz


//initialize uart
void InitUART(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    UARTStdioConfig(0, 115200, 16000000);
}
//initialize esp
void InitESPUART(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
	GPIOPinConfigure(GPIO_PB0_U1RX);
	GPIOPinConfigure(GPIO_PB1_U1TX);
	GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	UARTClockSourceSet(UART1_BASE, UART_CLOCK_PIOSC);
	UARTConfigSetExpClk(UART1_BASE, 16000000, 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
	UARTEnable(UART1_BASE);

}
// issue command to ESP8266
void SendATCommand(char *cmd)
{
	while(UARTBusy(UART1_BASE));
	while(*cmd != '\0')
	{
		UARTCharPut(UART1_BASE, *cmd++);
	}
	UARTCharPut(UART1_BASE, '\r'); //CR
	UARTCharPut(UART1_BASE, '\n'); //LF

}

int recvString(char *target, char *data, int timeout, bool check)
{
	int i=0;
	char a;
	unsigned long start = millis();

    while (millis() - start < timeout)
    {
    	while(UARTCharsAvail(UART1_BASE))
    	{
              a = UARTCharGet(UART1_BASE);
              if(a == '\0') continue;
              data[i]= a;
              i++;
    	}

    	if(check)
    	{
    		if (SearchIndexOf(data, target) != -1)
    		{
    			break;
    		}
    	}
    }

    return 0;
}

bool recvFind(char *target, int timeout,bool check)
{
	recvString(target, ReceivedData, timeout, check);

	if (SearchIndexOf(ReceivedData, target) != -1)
	{
		return true;
	}
	return false;
}

bool recvFindAndFilter(char *target, char *begin, char* end, char *data, int timeout)
{
	recvString(target, ReceivedData, timeout, true);

    if (SearchIndexOf(ReceivedData, target) != -1)
    {
         int index1 = SearchIndexOf(ReceivedData, begin);
         int index2 = SearchIndexOf(ReceivedData, end);

         if (index1 != -1 && index2 != -1)
         {
             index1 += strlen(begin);
             Substring(ReceivedData,data,index1, index2);
             return true;
         }
     }
     data = "";
     return false;
}
//command to check if working
bool ATesp(void)
{
	memset(ReceivedData, 0, sizeof(ReceivedData));
	SendATCommand("AT");
	return recvFind("OK",5000, true);
}
//restart command
bool RSTesp(void)
{
	memset(ReceivedData, 0, sizeof(ReceivedData));
	SendATCommand("AT+RST");
	return recvFind("OK",5000, false);
}
//
bool CWMODEesp(void)
{
	memset(ReceivedData, 0, sizeof(ReceivedData));
	SendATCommand("AT+CWMODE=1");
	return recvFind("OK",5000, true);
}
//join  access point
bool CWJAPesp(void)
{
	memset(ReceivedData, 0, sizeof(ReceivedData));
	SendATCommand("AT+CWJAP=\"zubinwifi\",\"12345678\""); //Your Wifi: NetworkName, Password
	return recvFind("OK",10000, true);
}
//command to quit access point
bool CWQAPesp(void)
{
	memset(ReceivedData, 0, sizeof(ReceivedData));
	SendATCommand("AT+CWQAP");
	return recvFind("OK",10000, true);
}
//command to set multiple connection
bool CIPMUXesp(void)
{
	memset(ReceivedData, 0, sizeof(ReceivedData));
	SendATCommand("AT+CIPMUX=0");
	return recvFind("OK",5000, true);
}

bool ATGMResp(char *version)
{
	memset(ReceivedData, 0, sizeof(ReceivedData));
	SendATCommand("AT+GMR");
	return recvFindAndFilter("OK", "\r\r\n", "\r\n\r\nOK", version,10000);
}
//command to configure wifi mode
bool aCWMODEesp(char *list)
{
	memset(ReceivedData, 0, sizeof(ReceivedData));
	SendATCommand("AT+CWMODE=?");
	return recvFindAndFilter("OK", "+CWMODE:(", ")\r\n\r\nOK", list,10000);
}

bool aCWLAPesp(char *list)
{
	memset(ReceivedData, 0, sizeof(ReceivedData));
	SendATCommand("AT+CWLAP");
	return recvFindAndFilter("OK","\r\r\n", "\r\n\r\nOK", list,15000);
}

bool aCIFSResp(char *list)
{
	memset(ReceivedData, 0, sizeof(ReceivedData));
	SendATCommand("AT+CIFSR");
	return recvFindAndFilter("OK","\r\r\n", "\r\n\r\nOK", list,15000);
}

bool CIPSTOesp(void)
{
	memset(ReceivedData, 0, sizeof(ReceivedData));
	SendATCommand("AT+CIPSTO=10000");
	return recvFind("OK",2000, true);
}
//Command to Set up TCP connection with a server
bool CIPSTARTesp(void)
{
	memset(ReceivedData, 0, sizeof(ReceivedData));
	SendATCommand("AT+CIPSTART=\"TCP\",\"192.168.159.1\",9998"); //Server IP and Port: such as 192.255.0.100, 9999
	return recvFind("OK",2000, true);
}
//Command to Close TCP Connection with a Server
bool CIPCLOSEesp(void)
{
	memset(ReceivedData, 0, sizeof(ReceivedData));
	SendATCommand("AT+CIPCLOSE");
	return recvFind("OK",5000, true);
}
//command to send text to server
bool CIPSENDesp(char *text)
{
	int len = strlen(text)+2;

	itoa(len,str);

	char* AT_CMD_SEND = "AT+CIPSEND=";
	char CMD_TEXT[128];
	strcpy(CMD_TEXT,AT_CMD_SEND);
	strcat(CMD_TEXT,str);

	memset(ReceivedData, 0, sizeof(ReceivedData));
	SendATCommand(CMD_TEXT);
	delay(5);
	SendATCommand(text);
	return recvFind("SEND OK",2000, true);
}
//reset  modules
void HardwareReset()
{
	GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_0); //Output
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_4, 0x00); //LOW ->Reset to ESP8266
    delay(50);
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_0); //Output ->// Open drain; reset -> GND
    delay(10);
    GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_0); //Input ->// Back to high-impedance pin state
    delay(3000);
}
//function to process command requests
void ProcessCommand(char *CommandText)
{
	long Status;
	char *array[10];
	int i=0;

	array[i] = strtok(CommandText,":");

	while(array[i]!=NULL)
	{
		array[++i] = strtok(NULL,":");
	}

	memset(COMMAND, 0, sizeof(COMMAND));
	strncpy(COMMAND, array[1], (strlen(array[1])-1));

	UARTprintf("CMD->%s\n",COMMAND);

	Status = CmdLineProcess(COMMAND);

	if(Status == CMDLINE_BAD_CMD)
	{
		UARTprintf("Bad command!\n");
	}
}

void QuitProcess(void)
{
	process = false;
}

///////// From here

// LOCK_F and CR_F - used for unlocking PORTF pin 0
#define LOCK_F (*((volatile unsigned long *)0x40025520))
#define CR_F   (*((volatile unsigned long *)0x40025524))
//initial setup
void setup(void)
{
	SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

	ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
	ADCSequenceStepConfigure(ADC0_BASE,1,0,ADC_CTL_CH5|ADC_CTL_IE|ADC_CTL_END);
	ADCSequenceEnable(ADC0_BASE, 1);

	ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_PROCESSOR, 0);
	ADCSequenceStepConfigure(ADC0_BASE,2,0,ADC_CTL_CH4|ADC_CTL_IE|ADC_CTL_END);
	ADCSequenceEnable(ADC0_BASE, 2);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_2);
}

//confiure ports and pins
void switchPinConfig(void)
{
	// GPIO PORTA Pin 5 and Pin6
	GPIODirModeSet(GPIO_PORTD_BASE,GPIO_PIN_2|GPIO_PIN_3,GPIO_DIR_MODE_IN); // Set Pin-4 of PORT F as Input. Modifiy this to use another switch
	GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_2|GPIO_PIN_3);
	GPIOPadConfigSet(GPIO_PORTD_BASE,GPIO_PIN_2|GPIO_PIN_3,GPIO_STRENGTH_12MA,GPIO_PIN_TYPE_STD_WPU);
}
// decaration and initilization of parameters corresponding to sensor values,correction and thresholds for monitoring
uint32_t sensor1[1];
uint32_t sensor2[1];

int32_t v1,v2, co2_device1, co2_device2,co2_level1;
int32_t co2_correction = 140;

int32_t warning_thresh = 600;
int32_t alert_thresh   = 800;

///////// To here

int main(void)
{
	bool TaskStatus;

	SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC |   SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x00);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0x00);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x00);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_0);
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_0, 0x00);

    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x02);

    timerInit(); //initialize timer

    HardwareReset();
    //setup and initialize modules and UART
	InitUART();
	InitESPUART();


	setup();

	switchPinConfig();



//begin execution
	UARTprintf("Execute!\n");

	delay(1000);
//check working of wifi module,restart and configure wifi mode
	TaskStatus = ATesp();
	TaskStatus = RSTesp();
	TaskStatus = CWMODEesp();

	while(true)
	{  //connect to AP
		UARTprintf("Trying to connect wi-fi\n");
		TaskStatus = CWJAPesp();
		if(TaskStatus)
		{
			UARTprintf("Connection is established!\n");
			break;
		}
	}

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x00);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0x04);
//setup multiple connections
	TaskStatus = CIPMUXesp();

	while(true)
	{//connect to server and communicate with it
		TaskStatus = ProcessRoutine();
		if(TaskStatus)
		{
			//close connection on failure/disconnect
			CIPCLOSEesp();
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x00); //Turn off Green Led
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0x04); //Turn on Blue Led
		}
		else
		{ //quit Access Point after completion
			delay(10);
			CWQAPesp();
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0x00); //Turn off Blue Led
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x02); //Turn on Red Led
		}

		//////
		///
		while (true) {
			ADCIntClear(ADC0_BASE, 1);
			ADCProcessorTrigger(ADC0_BASE, 1);
			ADCIntClear(ADC0_BASE, 2);
			ADCProcessorTrigger(ADC0_BASE, 2);
			//CIPSENDesp("Hello where 1");
			//SysCtlDelay(100000);
			while(!ADCIntStatus(ADC0_BASE, 1, false) && !ADCIntStatus(ADC0_BASE, 2, false))
			{
			}
			//CIPSENDesp("Hello where 2");
			ADCSequenceDataGet(ADC0_BASE, 1, sensor1);
			ADCSequenceDataGet(ADC0_BASE, 2, sensor2);
			v1 = sensor1[0],v2=sensor2[0];
			co2_device1 = v1 - co2_correction;
			co2_device2 = v2 - co2_correction;
			itoa(co2_device1, mystr);
			//CIPSENDesp("Hello where");
			//CIPSENDesp(mystr);
			SysCtlDelay(10000000);
		}
		///



		//////
	}

}

bool ProcessRoutine()
{
	bool status;
	process = true;

	UARTprintf("Waiting Server...\n");

	while(true)
	{
		status = CIPSTARTesp(); //start TCP Connection with server
		if(status)
		{
			UARTprintf("Communication is established!\n");
			break;
		}

		delay(50);
	}

	/* Getting data from the sensor and calibrating it to get
	 * amount of CO2 in ppm
	 */
	while (true) {
		ADCIntClear(ADC0_BASE, 1);
		ADCProcessorTrigger(ADC0_BASE, 1);
		ADCIntClear(ADC0_BASE, 2);
		ADCProcessorTrigger(ADC0_BASE, 2);
		//CIPSENDesp("Hello where 1");
		//SysCtlDelay(100000);
		while(!ADCIntStatus(ADC0_BASE, 1, false) && !ADCIntStatus(ADC0_BASE, 2, false))
		{
		}
		//CIPSENDesp("Hello where 2");
		ADCSequenceDataGet(ADC0_BASE, 1, sensor1);
		ADCSequenceDataGet(ADC0_BASE, 2, sensor2);
		v1 = sensor1[0],v2=sensor2[0];
		co2_device1 = v1 - co2_correction;
		co2_device2 = v2 - co2_correction;

		if (co2_device1 > alert_thresh) {											// Alert level
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x02);	// LED is Red
			GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_2,31);							// Start the buzzer
		}
		else if (co2_device1 >= warning_thresh) {									// Warning level
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x0a);	// LED is Yellow
			GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_2,0);
		}
		else {																		// Normal
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x08);	// LED is Green
			GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_2,0);
		}

		itoa(co2_device1, mystr);
		//CIPSENDesp("Hello where");
		strcat(mystr, " 1");			// Sending data to server along with sensor/device id
		CIPSENDesp(mystr);
		SysCtlDelay(10000000);			// ~1 sec delay
	}


	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0x00);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x08); //TurnOn Green Led-> READY

	int i=0;
	char a;
	memset(ReceivedData, 0, sizeof(ReceivedData)); //Clear
	unsigned long start;

	while(process)
	{
		if(UARTCharsAvail(UART1_BASE))
		{
			if (SearchIndexOf(ReceivedData, "+IPD,") != -1)
			{
				i=0;
				memset(ReceivedData, 0, sizeof(ReceivedData));

				start = millis();

			    while (millis() - start < 5000)
			    {
			    	while(UARTCharsAvail(UART1_BASE))
			    	{
			    		a = UARTCharGet(UART1_BASE);
			    		if(a == '\0') continue;
			    		ReceivedData[i]= a;
			    		i++;
			    	}

		    		if (SearchIndexOf(ReceivedData, "\n") != -1)
		    		{
		    			break;
		    		}
			    }

				ProcessCommand(ReceivedData);

				i=0;
				memset(ReceivedData, 0, sizeof(ReceivedData));
			}
			else
			{
				a = UARTCharGet(UART1_BASE);
				if(a == '\0') continue;
				ReceivedData[i]= a;
				i++;
			}
		}
	}

	return true;
}

//helper functions
char *Substring(char *src, char *dst, int start, int stop)
{
	int len = stop - start;
	strncpy(dst, src + start, len);

	return dst;
}

int SearchIndexOf(char src[], char str[])
{
   int i, j, firstOcc;
   i = 0, j = 0;

   while (src[i] != '\0')
   {

      while (src[i] != str[0] && src[i] != '\0')
         i++;

      if (src[i] == '\0')
         return (-1);

      firstOcc = i;

      while (src[i] == str[j] && src[i] != '\0' && str[j] != '\0')
      {
         i++;
         j++;
      }

      if (str[j] == '\0')
         return (firstOcc);
      if (src[i] == '\0')
         return (-1);

      i = firstOcc + 1;
      j = 0;
   }

   return (-1);
}

char* itoa(int i, char b[])
{
    char const digit[] = "0123456789";
    char* p = b;
    if(i<0){
        *p++ = '-';
        i *= -1;
    }
    int shifter = i;
    do{
        ++p;
        shifter = shifter/10;
    }while(shifter);
    *p = '\0';
    do{
        *--p = digit[i%10];
        i = i/10;
    }while(i);
    return b;
}

void ftoa(float f,char *buf)
{
    int pos=0,ix,dp,num;
    if (f<0)
    {
        buf[pos++]='-';
        f = -f;
    }
    dp=0;
    while (f>=10.0)
    {
        f=f/10.0;
        dp++;
    }
    for (ix=1;ix<8;ix++)
    {
            num = f;
            f=f-num;
            if (num>9)
                buf[pos++]='#';
            else
                buf[pos++]='0'+num;
            if (dp==0) buf[pos++]='.';
            f=f*10.0;
            dp--;
    }
}
