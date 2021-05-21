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

#define AXIS_X_DATA_POINTS 48
#define DATA_BUFFER_SIZE 32

#define GRAPH_POS_X 16
#define GRAPH_POS_Y 32
#define GRAPH_WIDTH 288 /* Must be multiple of AXIS_X_DATA_POINTS */
#define GRAPH_HEIGHT 112
#define AXIS_X_SPACING (GRAPH_WIDTH/AXIS_X_DATA_POINTS)

struct GraphData {
    uint32_t prevDataX;
    uint32_t prevDataY;
    uint32_t data[AXIS_X_DATA_POINTS];

    uint32_t density;

    uint32_t y_max;
    uint32_t y_estop;
    uint32_t graphHead;

    float axis_y_scale;
} Graph_RPM;

void GraphData_init(struct GraphData *data, uint32_t density, uint32_t estop) {
    data->density = density;
    data->y_estop = estop;
    data->graphHead = 0;
    data->y_max = 1;
    data->axis_y_scale = GRAPH_HEIGHT / data->y_max;
}

// Sensor Data Buffer
float dataBuffer[DATA_BUFFER_SIZE];
uint32_t dataHead = 0;
uint32_t dataTail = 0;

uint32_t getGraphY(float y, float scale) {
    return (GRAPH_HEIGHT + GRAPH_POS_Y - (y * scale));
}

void drawDataPoint(struct GraphData* graph, uint32_t dx, uint32_t dy) {
    GrLineDraw(&sContext, graph->prevDataX, getGraphY(graph->prevDataY, graph->axis_y_scale), dx, getGraphY(dy, graph->axis_y_scale));
    graph->prevDataX = dx;
    graph->prevDataY = dy;
}

void drawAllGraphData(struct GraphData *graph) {
    graph->prevDataX = GRAPH_POS_X;
    graph->prevDataY = graph->data[0];

    uint32_t i;
    for (i = 0; i < graph->graphHead; i++) {
        drawDataPoint(graph, GRAPH_POS_X + (i * AXIS_X_SPACING), graph->data[i]);
    }
}

void drawGraphBorder(void) {
    GrContextForegroundSet(&sContext, ClrWhite);
    GrLineDraw(&sContext, GRAPH_POS_X, GRAPH_POS_Y, GRAPH_POS_X, GRAPH_HEIGHT + GRAPH_POS_Y);
    GrLineDraw(&sContext, GRAPH_POS_X, GRAPH_HEIGHT + GRAPH_POS_Y, GRAPH_WIDTH + GRAPH_POS_X, GRAPH_HEIGHT + GRAPH_POS_Y);

    // x axis notches
    uint32_t i;
    for (i = 0; i < AXIS_X_DATA_POINTS; i++) {
        GrLineDraw(&sContext, GRAPH_POS_X + (AXIS_X_SPACING*i), GRAPH_HEIGHT + GRAPH_POS_Y - 2, GRAPH_POS_X + (AXIS_X_SPACING*i), GRAPH_HEIGHT + GRAPH_POS_Y + 2);
    }
}


void shiftGraphDataLeft(struct GraphData* graph) {
    uint32_t i;
    for (i = 0; i < AXIS_X_DATA_POINTS; i++) {
        graph->data[i] = graph->data[i+1];
    }
}

void clearGraph(struct GraphData *graph, float newData) {

    //static char dataStr[32];
    //static char dataPeak[32];

    GrContextForegroundSet(&sContext, ClrBlack);
    drawAllGraphData(graph);

    if (newData > graph->y_max) {
        GrLineDraw(&sContext, GRAPH_POS_X, getGraphY(graph->y_estop, graph->axis_y_scale), GRAPH_WIDTH + GRAPH_POS_X, getGraphY(graph->y_estop, graph->axis_y_scale));
    }

    //GrStringDraw(&sContext, dataStr, 32, GRAPH_POS_X + 16, GRAPH_POS_Y + GRAPH_HEIGHT + 16, 1);
    //GrStringDraw(&sContext, dataPeak, 32, GRAPH_POS_X + 16, GRAPH_POS_Y + GRAPH_HEIGHT + 32, 1);
}

void drawGraph(struct GraphData *graph) {
    // Draw the data
    GrContextForegroundSet(&sContext, ClrRed);
    GrLineDraw(&sContext, GRAPH_POS_X, getGraphY(graph->y_estop, graph->axis_y_scale), GRAPH_WIDTH + GRAPH_POS_X, getGraphY(graph->y_estop, graph->axis_y_scale));

    GrContextForegroundSet(&sContext, ClrYellow);
    drawAllGraphData(graph);
    drawGraphBorder();

    /*
        sprintf(&dataStr, "Current: %d", data[graphHead]);
        sprintf(&dataPeak, "Peak: %d", y_max);
        GrStringDraw(&sContext, dataStr, 32, GRAPH_POS_X + 16, GRAPH_POS_Y + GRAPH_HEIGHT + 16, 1);
        GrStringDraw(&sContext, dataPeak, 32, GRAPH_POS_X + 16, GRAPH_POS_Y + GRAPH_HEIGHT + 32, 1);
    */
}

void updateGraph(struct GraphData *graph, float newData) {

    if (graph->prevDataX == 0 && graph->prevDataY == 0) {
        graph->prevDataX = GRAPH_POS_X + (graph->graphHead * AXIS_X_SPACING);
        graph->prevDataY = getGraphY(newData, graph->axis_y_scale);
    }

    clearGraph(graph, newData);

    // If received data is greater than current y limits, re-scale graph
    if (newData > graph->y_max) {
        // Rescale
        graph->axis_y_scale = graph->axis_y_scale * (graph->y_max/newData);
        graph->y_max = newData;
    }

    // If graph x axis is full, shift all data back one index
    if (graph->graphHead == AXIS_X_DATA_POINTS) {
        shiftGraphDataLeft(graph);
        --graph->graphHead;
    }

    // Copy data from the sensor buffer, to the graph buffer
    graph->data[graph->graphHead] = newData;


    // Increment the current index in the data array
    if (graph->graphHead < AXIS_X_DATA_POINTS) ++graph->graphHead;

    drawGraph(graph);

}

void addDataToBuffer(float y) {
    dataBuffer[dataHead] = y;

    ++dataHead;
    if (dataHead > DATA_BUFFER_SIZE) {
        dataHead = 0;
    }
}

void GUI_Graphing(UArg arg0, UArg arg1)
{
    // Initialise graph data
    GraphData_init(&Graph_RPM, 1, 1200);

    // Draw Graph Borders
    drawGraphBorder();

    // Random Data Generator
    time_t t;
    uint32_t range = 15;
    srand((unsigned) time(&t));

    // Forever
    while (1) {

        // If data is in the buffer, ready to be drawn
        while (dataHead != dataTail) {

            updateGraph(&Graph_RPM, dataBuffer[dataTail]);

            dataBuffer[dataTail] = 0;

            // Increment sensor buffer tail
            ++dataTail;
            if (dataTail > DATA_BUFFER_SIZE) {
                dataTail = 0;
            }

            // Increase random data range
            range += 5;
        }

        addDataToBuffer(rand() % (range));
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
    Task_construct(&task0Struct, (Task_FuncPtr)GUI_Graphing, &taskParams, NULL);

    // Turn on user LED
    GPIO_write(Board_LED0, Board_LED_ON);

    //tContext sContext;
    Kentec320x240x16_SSD2119Init(SYS_CLK_SPEED);
    GrContextInit(&sContext, &g_sKentec320x240x16_SSD2119);
    TouchScreenInit(SYS_CLK_SPEED);
    TouchScreenCallbackSet(WidgetPointerMessage);


    char str[32];
    sprintf(&str, "%dx%d", GRAPH_WIDTH, GRAPH_HEIGHT);
    FrameDraw(&sContext, str);

    System_printf("Starting the example\nSystem provider is set to SysMin. "
                  "Halt the target to view any SysMin contents in ROV.\n");
    /* SysMin will only print to the console when you call flush or exit */
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);
}
