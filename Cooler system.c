/*
 * Cooler_system.c
 * Created: 01/25/2021 10:50:44 AM
 *  Author: REHAB
 */ 

#include <avr/interrupt.h>

#include "ADC.h"
#include "INTERRUPT.h"
#include "TIMER0.h"

#include "LCD.h"
#include "LED.h"
#include "BUZZER.h"
#include "SEVEN_SEG.h"
#include "LM35_TEMP_SENSOR.h"

volatile uint8 Temperature=0;
int main(void)
{
    LCDInit();
	LedInit(LED0);
	BuzzerInit(BUZZER0);
	SevenSegBCDInit(BOTH);
	Timer0SetOutputWavePin();
	
	ADCCustomInitSingleConversionMode(ADC_PRESC_64,LM35_SENSOR0,AVCC);
	Timer0Init(TIMER0_FAST_PWM_MODE,TIMER0_INTERNAL_CLK_PRESC_1024,0);
	
	ADCInterruptInit();
	Timer0InterruptInit(TIMER0_OVERFLOW);
	
	LCDDisplayString(1,1,"Temp =   c");
	LCDDisplayString(2,1,"Duty = ");
	while(1)
    {
		 SevenSegBCDWrite(SEG1,Temperature%10);
		 _delay_ms(1);
		 SevenSegBCDWrite(SEG2,Temperature/10);
		 _delay_ms(1);
    }
}

ISR(ADC_vect)
{
	Temperature=LM35TempSensorCalculateTemp(AVCC);
	float32 VoltageValue=((Temperature-35)*0.16)+2.5;
	uint8 Duty=Timer0ReturnDutyOfAnalogWrite(VoltageValue);
	static uint8 Flag=0;
	if((Temperature>35) && (Flag==0))
	{
		BuzzerOn(BUZZER0);
		LedOn(LED0);
		Timer0AnalogWrite(VoltageValue);
		Timer0SelectCompareMatchOutputWaveMode(NON_INVERTING_OC0_MODE);	
		Flag++;	
	}
	else if((Temperature>35) && (Flag==1))
	{
		BuzzerOn(BUZZER0);
		LedOn(LED0);
		Timer0AnalogWrite(VoltageValue);
	}	
	else
	{
		BuzzerOff(BUZZER0);
		LedOff(LED0);
		Timer0SelectCompareMatchOutputWaveMode(DISCONNECT_OC0);
		Duty=0;
		Flag=0;
		LCDDisplayChar(2,9,' ');
		LCDDisplayChar(2,10,' ');			
	}
	LCDDisplayNumber(1,8,Temperature);
	LCDDisplayNumber(2,8,Duty);
}

ISR(TIMER0_OVF_vect)
{
	static uint32 NumberOfOverflow=0;
	NumberOfOverflow++;
	if(NumberOfOverflow==Timer0Delay_sec_Normal(1,1024))
	{
		LM35TempSensorRead(LM35_SENSOR0);
		NumberOfOverflow=0;
	}
	
}