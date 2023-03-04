/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"

/* Demo application includes. */
#include "serial.h"
#include "GPIO.h"
#include "semphr.h"

/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )


/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */

static void prvSetupHardware( void );
void Button_1_Monitor(void * ptr);
void Button_2_Monitor(void * ptr);
void Periodic_Transmitter(void * ptr);
void Uart_Receiver(void * ptr) ;
void Load_1_Simulation(void * ptr) ;
void Load_2_Simulation(void * ptr);
/*-----------------------------------------------------------*/

TaskHandle_t B1_Handler;
TaskHandle_t B2_Handler;
TaskHandle_t TX_Handler;
TaskHandle_t RX_Handler;
TaskHandle_t L1_PERIOD;
TaskHandle_t L2_PERIOD;
xQueueHandle ptrToMSG;
/*-----------------------------------------------------------*/

float CPU_Load=0;
int System_time=0;
int B1_In = 0, B1_Out = 0, B1_Total = 0;
int B2_In = 0, B2_Out = 0, B2_Total = 0;
int Tx_In = 0, Tx_Out = 0, Tx_Total = 0;
int Rx_In = 0, Rx_Out = 0, Rx_Total = 0;
int L1_In = 0, L1_Out = 0, L1_Total = 0;
int L2_In = 0, L2_Out = 0, L2_Total = 0;
/*-----------------------------------------------------------*/



char B1_Last_Level;
char B1_current_Level;
char B2_Last_Level;
char B2_current_Level;
char RX_buffer[16]; 
/*-----------------------------------------------------------*/

	

int main( void )
{
	
	prvSetupHardware();
	ptrToMSG=xQueueCreate(6,16);
	xTaskPeriodicCreate(Button_1_Monitor,"Task_1",60,(void*)0,&B1_Handler,50);
  xTaskPeriodicCreate(Button_2_Monitor,"Task_2",60,(void*)0,&B2_Handler,50);
	xTaskPeriodicCreate(Periodic_Transmitter,"Task_3",60,(void*)0,&TX_Handler,100);
  xTaskPeriodicCreate(Uart_Receiver,"Task_4",60,(void*)0,&RX_Handler,20);
	xTaskPeriodicCreate(Load_1_Simulation,"Task_5",60,(void*)0,&L1_PERIOD,10);
  xTaskPeriodicCreate(Load_2_Simulation,"Task_6",60,(void*)0,&L2_PERIOD,100);
    /* Create Tasks here */
	vTaskStartScheduler();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationTickHook(void)
{
	GPIO_write(PORT_0,PIN9,PIN_IS_HIGH);
	GPIO_write(PORT_0,PIN9,PIN_IS_LOW);
}
/*-----------------------------------------------------------*/


void timer1Reset(void)
{
	T1TCR |= 0x2;
	T1TCR &= ~0x2;
}
/*-----------------------------------------------------------*/

static void configTimer1(void)
{
	T1PR = 1000;
	T1TCR |= 0x1;
}
/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();
	
	/* Config trace timer 1 and read T1TC to get current tick */
	configTimer1();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/

void Button_1_Monitor(void * ptr) 
{
	TickType_t WakeTime=xTaskGetTickCount();
	vTaskSetApplicationTaskTag( NULL, (TaskHookFunction_t) 1 );
	
	for(;;)
	{
	B1_current_Level=GPIO_read(PORT_0,PIN0);
	if(B1_current_Level==1&&B1_Last_Level==0) xQueueSendToBack(ptrToMSG,"B1 Raising Edge\n",0);

	else if(B1_current_Level==0&&B1_Last_Level==1) xQueueSendToBack(ptrToMSG,"B1 Falling Edge\n",0);

	B1_Last_Level=B1_current_Level;
		
	vTaskDelayUntil(&WakeTime,50);

	}
}
/*-----------------------------------------------------------*/

void Button_2_Monitor(void * ptr)
{
	TickType_t WakeTime=xTaskGetTickCount();
	vTaskSetApplicationTaskTag( NULL, (TaskHookFunction_t) 2 );
	
	for(;;)
	{
	B2_current_Level=GPIO_read(PORT_0,PIN1);
	if(B2_current_Level==1&&B2_Last_Level==0) xQueueSendToBack(ptrToMSG,"B2 Raising Edge\n",0);

	else if(B2_current_Level==0&&B2_Last_Level==1) xQueueSendToBack(ptrToMSG,"B2 Falling Edge\n",0);

	B2_Last_Level=B2_current_Level;
		
	vTaskDelayUntil(&WakeTime,50);
	}
}
/*-----------------------------------------------------------*/

void Periodic_Transmitter(void * ptr)  
{
	TickType_t WakeTime=xTaskGetTickCount();
	vTaskSetApplicationTaskTag( NULL, (TaskHookFunction_t) 3 );
	for(;;)
	{
		xQueueSendToBack(ptrToMSG,"Periodic String\n\n",0);
	  	vTaskDelayUntil(&WakeTime,100);
	}
}
/*-----------------------------------------------------------*/

void Uart_Receiver(void * ptr)  
{
	TickType_t WakeTime=xTaskGetTickCount();
	vTaskSetApplicationTaskTag( NULL, (TaskHookFunction_t) 4 );
	for(;;)
	{
  if(xQueueReceive(ptrToMSG,RX_buffer,0)==pdTRUE) vSerialPutString((signed char*)(RX_buffer),16);
	
	
	vTaskDelayUntil(&WakeTime,20);
	}
}

void Load_1_Simulation(void * ptr) 
{
	unsigned int i = 0;
	TickType_t WakeTime=xTaskGetTickCount();
	vTaskSetApplicationTaskTag( NULL, (TaskHookFunction_t) 5 );
	for(;;)
	{
	for(i=0;i<=11910;i++)
		{}
	vTaskDelayUntil(&WakeTime,10);	
	}
}
/*-----------------------------------------------------------*/

void Load_2_Simulation(void * ptr) 
{
	unsigned int i = 0;
	TickType_t WakeTime=xTaskGetTickCount();
	vTaskSetApplicationTaskTag( NULL, (TaskHookFunction_t) 6 );
	
	for(;;)
	{
	for(i=0;i<=28620;i++)
		{}
	vTaskDelayUntil(&WakeTime,100);	
	}
}
