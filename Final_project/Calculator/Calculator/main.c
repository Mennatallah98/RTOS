/*
 * Calculator.c
 *
 * Created: 01/04/2020 3:06:31 PM
 * Author : Mennatallah
 */ 

#include <avr/io.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#define F_CPU 8000000
#include <util/delay.h>
#include <avr/io.h>

//keypad defines 
#define KEYPAD_PORT PORTC
#define KEYPAD_DDR   DDRC
#define KEYPAD_PIN   PINC
//*******************************

void Set_PWM (void)
{
	//setting registers for timer
	 TCCR0 = (1<<WGM00)|(1<<WGM01)|(1<<COM00)|(1<<COM01)|(1<<CS00);
	 TCNT0 = (unsigned char)0;
	 OCR0  = (unsigned char)0;
}


xTaskHandle  Task_plw_id;
xTaskHandle  Task_plp_id;
xTaskHandle  Task_pb_id;
xTaskHandle  Task_calc_id;
xSemaphoreHandle xSemaphore;

int direction;
int x=0;
int counter=0;

void vTaskStartScheduler( void );



//LCD function
void LcdWrite_move (void);
void LcdWrite_blink (void);
void period_lcd_welcome(void *p);
void period_lcd_press(void *p);
//**************************************************

//LED functions
void LedRising(void);
void LedHigh(void);
void LedFalling(void);
void LedLow(void);
void period_blinking(void *p);
//******************************************************

//Keypad functions
unsigned char GetKeyPressed(void);
unsigned char KeyPad_AdjustKeyNumber(unsigned char button_number);
void Calculator (void *p);
//********************************************************************

int main(void)
{
	lcd_init();
	DDRB = 0Xff ; //  port B output  
	xTaskCreate(period_lcd_welcome, NULL, 100, NULL,3, &Task_plw_id );
	xTaskCreate(period_lcd_press,   NULL, 100, NULL,2, &Task_plp_id );
	xTaskCreate(period_blinking,    NULL, 100, NULL,1, &Task_pb_id );
	xSemaphoreCreateBinary(xSemaphore);
	vTaskStartScheduler();

	return 0;
}



//Functions
void LcdWrite_move (void )
{
	lcd_disp_string_xy("Welcome",0,x);

	if(x==0)
	{
		direction=0;
		x++;
		counter++;
	}
	else if(x==9)
	{
		direction=1;
		x--;
	}
	else
	{
		if(direction==0)
		{
			x++;
		}
		else if (direction==1)
		{
			x--;
		}			
	}

	
}

void LcdWrite_blink (void)
{
	lcd_disp_string_xy("Press any key to ",0,0);
	lcd_disp_string_xy("continue",1,4);	
}

void LedHigh(void)
{
	PORTB |= 0x08;	
}

void LedLow(void)
{
	PORTB &= 0xF7;
	
}

void LedRising(void)
{
	int x;
	for(x=0;x++;x<=10)
	{
		OCR0 = 25.5*x;
		_delay_ms(25);		
	}	
}

void LedFalling(void)
{
	int x;
	for(x=10;x--;x>=0)
	{
		OCR0 = 25.5*x;
		_delay_ms(25);
	}
}

unsigned char GetKeyPressed(void)
{
	 unsigned char r,c;
	KEYPAD_PORT|= 0X0F;
	
	for(c=0;c<4;c++)
	{
		KEYPAD_DDR &=~(0X7F);
		KEYPAD_DDR |=(0X40>>c);
		for(r=0;r<4;r++)
		{
			if(!(KEYPAD_PIN & (0X08>>r)))
			 {	
				if(xSemaphoreTake(xSemaphore,portMAX_DELAY)==pdTRUE)
				{
					return KeyPad_AdjustKeyNumber((r*4)+c+1);					
				}		 				
			 }
		}
	}
	
	return NULL;//Indicate No key pressed
}

unsigned char KeyPad_AdjustKeyNumber(unsigned char button_number)
{
	switch(button_number)
	{
		case 1: return '7';
		break;
		case 2: return '8';
		break;
		case 3: return '9';
		break;
		case 4: return '0'; // ASCII Code of %
		break;
		case 5: return '4';
		break;
		case 6: return '5';
		break;
		case 7: return '6';
		break;
		case 8: return '*'; /* ASCII Code of '*' */
		break;
		case 9: return '1';
		break;
		case 10: return '2';
		break;
		case 11: return '3';
		break;
		case 12: return '-'; /* ASCII Code of '-' */
		break;
		case 13: return 13;  /* ASCII of Enter */
		break;
		case 14: return '0';
		break;
		case 15: return '='; /* ASCII Code of '=' */
		break;
		case 16: return '+'; /* ASCII Code of '+' */
		break;
		default: return NULL;
	}

}
//************************************************************************************************************

//Tasks
void period_lcd_welcome (void *p)
{	
	portTickType xLastWakeTime;
	const portTickType xPeriod = 25;			
	xLastWakeTime = xTaskGetTickCount();
    portTickType xPeriod_off = 250;
	const portTickType time = portMAX_DELAY;
	xSemaphoreTake(xSemaphore,time);
	while(counter<=3)
	{
		vTaskDelayUntil( &xLastWakeTime, xPeriod );
		lcd_clrScreen();
		LcdWrite_move ();
	}
	lcd_clrScreen();
	xSemaphoreGive(xSemaphore);
	vTaskDelete(NULL);
}

void period_lcd_press(void *p)
{
	portTickType xLastWakeTime;
	const portTickType xPeriod_on  = 500;
	const portTickType xPeriod_off = 250;
	const portTickType time = portMAX_DELAY;	
	xLastWakeTime = xTaskGetTickCount();
	if(xSemaphoreTake(xSemaphore,time)==pdTRUE)
	{
		while((xLastWakeTime<=10000))
		{				
			LcdWrite_blink();
			vTaskDelayUntil( &xLastWakeTime, xPeriod_on );
			lcd_clrScreen();
			vTaskDelayUntil( &xLastWakeTime, xPeriod_off );
			if (GetKeyPressed() != NULL)
			{
				xSemaphoreGive(xSemaphore);
				vTaskDelete(NULL);
				xTaskCreate(Calculator,  NULL, 100, NULL,1, &Task_calc_id );
			}

			
		}
	}
	lcd_clrScreen();
	xSemaphoreGive(xSemaphore);
	vTaskDelete(NULL);
}

void period_blinking(void *p)
{
	portTickType xLastWakeTime;
	const portTickType xPeriod = 250;
	const portTickType time = portMAX_DELAY;
	xLastWakeTime = xTaskGetTickCount();
	if(xSemaphoreTake(xSemaphore,time)==pdTRUE)
	{
		while(1)
		{
			LedRising();
			vTaskDelayUntil( &xLastWakeTime, xPeriod);			
			LedHigh();
			vTaskDelayUntil( &xLastWakeTime, xPeriod );
			LedFalling();
			vTaskDelayUntil( &xLastWakeTime, xPeriod);			
			LedLow();
			vTaskDelayUntil( &xLastWakeTime, xPeriod );
		}
	}
	xSemaphoreGive(xSemaphore);
	vTaskDelete(NULL);	
}

void Calculator(void *p)
{
	if(xSemaphoreTake(xSemaphore,portMAX_DELAY)==pdTRUE)
	{
		unsigned char x= GetKeyPressed();
		lcd_gotoxy(0,0);
		lcd_displayChar(x);
	}
}

