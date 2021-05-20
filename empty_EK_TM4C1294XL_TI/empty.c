/*
 * Copyright (c) 2015, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== empty.c ========
 */
/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_memmap.h"

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <driverlib/gpio.h>
#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/knl/swi.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Queue.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>



#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "grlib/grlib.h"
#include "drivers/frame.h"
#include "drivers/kentec320x240x16_ssd2119_spi.h"
#include "drivers/pinout.h"
#include "grlib/widget.h"
#include "grlib/canvas.h"
#include "grlib/pushbutton.h"
#include "drivers/touch.h"
#include <driverlib/sysctl.h>
#include "utils/ustdlib.h"
#include <math.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include <driverlib/gpio.h>
#include <ti/sysbios/hal/Seconds.h>
#include <time.h>

// Graphics
tContext sContext;
uint8_t motorStartStop = 1;

/* Board Header file */
#include "Board.h"

#define TASKSTACKSIZE   1024

Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];


tCanvasWidget     g_sBackground;
tPushButtonWidget g_sStartStopBttn;

void StartStopBttnPress(tWidget *psWidget);

// The canvas widget acting as the background to the display.
Canvas(g_sBackground, WIDGET_ROOT, 0, &g_sStartStopBttn,
       &g_sKentec320x240x16_SSD2119, 10, 25, 300, (240 - 25 -10),
       CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);

RectangularButton(g_sStartStopBttn, &g_sBackground, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 50, 200, 100, 25,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrDarkBlue, ClrBlue, ClrWhite, ClrWhite,
                   g_psFontCmss16b, "Start", 0, 0, 0, 0, StartStopBttnPress);



void StartStopBttnPress(tWidget *psWidget)
{
    motorStartStop = !motorStartStop;

    if(motorStartStop)
    {
        //
        // Change the button text to indicate the new function.
        //
        PushButtonTextSet(&g_sStartStopBttn, "Stop");

        //
        // Repaint the pushbutton and all widgets beneath it (in this case,
        // the welcome message).
        //
        WidgetPaint((tWidget *)&g_sStartStopBttn);
    }
    else
    {
        //
        // Change the button text to indicate the new function.
        //
        PushButtonTextSet(&g_sStartStopBttn, "Start");

        WidgetPaint((tWidget *)&g_sStartStopBttn);
    }
}



//void ADC3_Init() //ADC0 on PE3
//{
//    //SysCtlPeripheralEnable( SYSCTL_PERIPH_ADC0 );
//    //SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOE );
//
//    //Makes GPIO an INPUT and sets them to be ANALOG
//    GPIOPinTypeADC( GPIO_PORTE_BASE, GPIO_PIN_3 );
//
//    //uint32_t ui32Base, uint32_t ui32SequenceNum, uint32_t ui32Trigger, uint32_t ui32Priority
//    ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_ALWAYS, 0);
//
//    //uint32_t ui32Base, uint32_t ui32SequenceNum, uint32_t ui32Step, uint32_t ui32Config
//    ADCSequenceStepConfigure( ADC0_BASE, 0, 0, ADC_CTL_IE | ADC_CTL_CH0 | ADC_CTL_END );
//
//    // Set oversampling
////    ADCHardwareOversampleConfigure(ADC0_BASE, 64);
//
//    ADCSequenceEnable( ADC0_BASE, 0 );
//
//    ADCIntClear( ADC0_BASE, 0 );
//
//    current = (Current*) malloc(sizeof(Current));
//}


/*
 *  ======== heartBeatFxn ========
 *  Toggle the Board_LED0. The Task_sleep is determined by arg0 which
 *  is configured for the heartBeat Task instance.
 */
#define SYS_CLK_SPEED 120000000
uint32_t axis_x_spacing;
#define AXIS_Y_SPACING 16
#define AXIS_X_DATA_POINTS 24
#define DATA_BUFFER_SIZE 32
#define PADDING_X 16
#define PADDING_Y 16

uint32_t axis_x_scale = 1;
float axis_y_scale = 1;

uint32_t prevDataX;
uint32_t prevDataY;
uint32_t dataBuffer[DATA_BUFFER_SIZE];
uint32_t data[AXIS_X_DATA_POINTS];
uint32_t y_max;

uint32_t dataHead = 0;
uint32_t dataTail = 0;

uint32_t graphHead = 0;

uint32_t graphHeight;
uint32_t graphWidth;

uint32_t getGraphY(float y) {
    return (graphHeight + PADDING_Y - (y * axis_y_scale));
}

void drawDataPoint(uint32_t dx, uint32_t dy) {

    GrLineDraw(&sContext, prevDataX, getGraphY(prevDataY), dx, getGraphY(dy));
    GrCircleFill(&sContext, dx, getGraphY(dy), 3);
    prevDataX = dx;
    prevDataY = dy;
}

void drawGraphBorder(void) {
    GrContextForegroundSet(&sContext, ClrWhite);
    GrLineDraw(&sContext, PADDING_X, PADDING_Y, PADDING_X, GrContextDpyHeightGet(&sContext) - PADDING_Y);
    GrLineDraw(&sContext, PADDING_X, GrContextDpyHeightGet(&sContext) - PADDING_Y, GrContextDpyWidthGet(&sContext) - PADDING_X, GrContextDpyHeightGet(&sContext) - PADDING_Y);

    uint32_t i;
    for (i = 0; i < AXIS_X_DATA_POINTS * axis_x_scale; i++) {
        GrLineDraw(&sContext, PADDING_X + (axis_x_spacing*i), GrContextDpyHeightGet(&sContext) - PADDING_Y - 2, PADDING_X + (axis_x_spacing*i), GrContextDpyHeightGet(&sContext) - PADDING_Y + 2);
    }

    for (i = 0; i < AXIS_X_DATA_POINTS * axis_x_scale; i++) {
        GrLineDraw(&sContext, PADDING_X + (axis_x_spacing*i), GrContextDpyHeightGet(&sContext) - PADDING_Y - 2, PADDING_X + (axis_x_spacing*i), GrContextDpyHeightGet(&sContext) - PADDING_Y + 2);
    }

    GrContextForegroundSet(&sContext, ClrYellow);
}

void redrawDataPoints(uint32_t from, uint32_t to) {
    prevDataX = PADDING_X;
    prevDataY = data[from];

    uint32_t i;
    for (i = from; i < to; i++) {
        drawDataPoint(PADDING_X + (i * axis_x_spacing), data[i]);
    }
}

void rescaleGraph(float y) {
    GrContextForegroundSet(&sContext, ClrBlack);
    redrawDataPoints(0, graphHead);

    axis_y_scale = y;

    GrContextForegroundSet(&sContext, ClrYellow);
    redrawDataPoints(0, graphHead);
    drawGraphBorder();
}

void shiftGraph(void) {

    GrContextForegroundSet(&sContext, ClrBlack);
    redrawDataPoints(0, AXIS_X_DATA_POINTS);

    uint32_t i;
    for (i = 0; i < AXIS_X_DATA_POINTS; i++) {
        data[i] = data[i+1];
    }

    GrContextForegroundSet(&sContext, ClrYellow);
    redrawDataPoints(0, AXIS_X_DATA_POINTS-1);
    drawGraphBorder();
}

void drawNextDataPoint(void) {

    if (graphHead == AXIS_X_DATA_POINTS) {
        shiftGraph();
        --graphHead;
    }

    if (prevDataX == 0 && prevDataY == 0) {
        prevDataX = PADDING_X + (graphHead * axis_x_spacing);
        prevDataY = data[0];
    }

    drawDataPoint(PADDING_X + (graphHead * axis_x_spacing), dataBuffer[dataTail]);

    data[graphHead] = dataBuffer[dataTail];
    dataBuffer[dataTail] = 0;

    ++dataTail;
    if (dataTail > DATA_BUFFER_SIZE) {
        dataTail = 0;
    }

    if (graphHead < AXIS_X_DATA_POINTS) ++graphHead;

}

void addDataPoint(float y) {

    dataBuffer[dataHead] = y;

    if (y > y_max) {
        rescaleGraph(axis_y_scale * (y_max/y));
        y_max = y;
    }

    ++dataHead;
    if (dataHead > DATA_BUFFER_SIZE) {
        dataHead = 0;
    }

}

void g_graphSetup(UArg arg0, UArg arg1)
{
    // Add the compile-time defined widgets to the widget tree.
    // WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sBackground);

    // Paint the widget tree to make sure they all appear on the display.
    // WidgetPaint(WIDGET_ROOT);

    drawGraphBorder();

    y_max = 1;
    axis_y_scale = (graphHeight - PADDING_Y) / y_max;

    uint32_t range = graphHeight - PADDING_Y;

    time_t t;
    srand((unsigned) time(&t));

    uint32_t cheapTimer = 0;
    while (1) {

        while (dataHead != dataTail) {
            drawNextDataPoint();
            range += 10;
        }

        ++cheapTimer;
        if (cheapTimer > 2000000) {
            cheapTimer = 0;
            addDataPoint(rand() % (range));
        }

        /*SysCtlDelay(100);
        GPIO_toggle(Board_LED0);

        WidgetMessageQueueProcess();*/

//        TouchScreenIntHandler
    }
}

/*
 *  ======== main ========
 */
int main(void)
{
    Task_Params taskParams;

    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();
    Board_initI2C();
    PinoutSet(false, false);


    /* Construct heartBeat Task  thread */
    Task_Params_init(&taskParams);
    taskParams.arg0 = 1000;
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task0Stack;
    Task_construct(&task0Struct, (Task_FuncPtr)g_graphSetup, &taskParams, NULL);

    // Turn on user LED
    GPIO_write(Board_LED0, Board_LED_ON);

    //tContext sContext;
    Kentec320x240x16_SSD2119Init(SYS_CLK_SPEED);
    GrContextInit(&sContext, &g_sKentec320x240x16_SSD2119);
    TouchScreenInit(SYS_CLK_SPEED);
    TouchScreenCallbackSet(WidgetPointerMessage);

    graphHeight = GrContextDpyHeightGet(&sContext) - (PADDING_X*2);
    graphWidth = GrContextDpyWidthGet(&sContext) - (PADDING_Y*2);

    axis_x_spacing = graphWidth / AXIS_X_DATA_POINTS;

    char str[32];
    sprintf(&str, "%dx%d", graphWidth, graphHeight);
    FrameDraw(&sContext, str);

    System_printf("Starting the example\nSystem provider is set to SysMin. "
                  "Halt the target to view any SysMin contents in ROV.\n");
    /* SysMin will only print to the console when you call flush or exit */
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);
}