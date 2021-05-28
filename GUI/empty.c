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
#include "grlib/slider.h"
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
uint8_t eStop = 0;
uint8_t tabNo = 1;
uint16_t SPEED_USER_LIMIT = 5;
uint16_t CURRENT_USER_LIMIT = 100;
uint16_t ACCEL_USER_LIMIT = 50;
uint32_t clockTicks = 0;


/* Time Values */
struct tm * timeinfo;

/* Board Header file */
#include "Board.h"

#define TASKSTACKSIZE   1024
#define SPEED_LIMIT 100
#define CURRENT_LIMIT 2000
#define ACCEL_LIMIT 500

Task_Struct task0Struct, task1Struct;
Char task0Stack[TASKSTACKSIZE], task1Stack[TASKSTACKSIZE];;
Event_Struct evtStruct;
Event_Handle evtHandle;


tCanvasWidget     g_sBackground;
tPushButtonWidget g_sStartStopBttn;
tCanvasWidget     g_sEstopText;
tCanvasWidget     g_sEstopLight;
tCanvasWidget     g_sDayAlert;
tCanvasWidget     g_sDate;
tPushButtonWidget g_sSwitcher;

tCanvasWidget     g_sSpeedCanvas;
tPushButtonWidget g_sSpeedSubBttn;
tPushButtonWidget g_sSpeedAddBttn;
tCanvasWidget     g_sCurrentCanvas;
tPushButtonWidget g_sCurrentSubBttn;
tPushButtonWidget g_sCurrentAddBttn;
tCanvasWidget     g_sAccelCanvas;
tPushButtonWidget g_sAccelSubBttn;
tPushButtonWidget g_sAccelAddBttn;
tCanvasWidget     g_sEstopText;


void StartStopBttnPress(tWidget *psWidget);
void onSpeedChange(tWidget *psWidget);
void onCurrentChange(tWidget *psWidget);
void onAccelChange(tWidget *psWidget);
void DrawHomeScreen();
void RemoveHomeScreen();
void eStopFxn(UArg arg0, UArg arg1);
void onDayNightChange();
void onTabSwap();
void initTime();
void getCurrentTime();

// The canvas widget acting as the background to the display.
Canvas(g_sBackground, 0, &g_sStartStopBttn, 0,
       &g_sKentec320x240x16_SSD2119, 10, 25, 300, (240 - 25 -10),
       CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);

CircularButton(g_sStartStopBttn, &g_sBackground, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 275, 50, 25,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrGreen, ClrRed, ClrWhite, ClrWhite,
                   g_psFontCmss16b, "Start", 0, 0, 0, 0, StartStopBttnPress);
// E-Stop Indicator
Canvas(g_sEstopText, 0, 0, 0,
       &g_sKentec320x240x16_SSD2119, 250, 80, 50, 40,
       CANVAS_STYLE_TEXT| CANVAS_STYLE_TEXT_OPAQUE, ClrBlack, 0, ClrWhite, g_psFontCmss16b, "e-Stop", 0, 0);

Canvas(g_sEstopLight, 0, 0, 0,
       &g_sKentec320x240x16_SSD2119, 240, 95, 10, 10,
       CANVAS_STYLE_FILL, ClrGreen, ClrWhite, 0, 0, 0, 0, 0);

// Day Alert
static char dayNight[10] = "Day";
Canvas(g_sDayAlert, 0, 0, 0,
       &g_sKentec320x240x16_SSD2119, 250, 100, 50, 40,
       CANVAS_STYLE_TEXT| CANVAS_STYLE_TEXT_OPAQUE, ClrBlack, 0, ClrWhite, g_psFontCmss16b, dayNight, 0, 0);

// Date
static char currentTime[30] = "00:00:00 01/01/1970";
Canvas(g_sDate, 0, 0, 0,
       &g_sKentec320x240x16_SSD2119, 10, 200, 180, 40,
       CANVAS_STYLE_TEXT| CANVAS_STYLE_TEXT_OPAQUE, ClrBlack, 0, ClrWhite, g_psFontCmss16b, currentTime, 0, 0);

// Panel Switcher
RectangularButton(g_sSwitcher, 0, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 260, 200, 50, 30,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrBlue, ClrBlue, ClrWhite, ClrWhite,
                   g_psFontCmss16b, "Graph", 0, 0, 0, 0, 0); // ADD ONTABSWAP FUNCTION WHEN CONNECTING. NOT CONNECTED YET BECAUSE WILL BORK SYSTEM

// Limit Titles
Canvas(g_sLimitTitle, 0, 0, 0,
       &g_sKentec320x240x16_SSD2119, 60, 20, 50, 40,
       CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_OPAQUE, ClrBlack, 0, ClrWhite, g_psFontCmss16b,"Limits", 0, 0);

Canvas(g_sSpeedTitle, 0, 0, 0,
       &g_sKentec320x240x16_SSD2119, 150, 45, 50, 40,
       CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_OPAQUE, ClrBlack, 0, ClrWhite, g_psFontCmss16b, "Speed", 0, 0);
Canvas(g_sCurrentTitle, 0, 0, 0,
       &g_sKentec320x240x16_SSD2119, 150, 95, 50, 40,
       CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_OPAQUE, ClrBlack, 0, ClrWhite, g_psFontCmss16b, "Curr", 0, 0);
Canvas(g_sAccelTitle, 0, 0, 0,
       &g_sKentec320x240x16_SSD2119, 150, 145, 50, 40,
       CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_OPAQUE, ClrBlack, 0, ClrWhite, g_psFontCmss16b, "Accel", 0, 0);


// Speed
static char Speed[10] = "5%";
Canvas(g_sSpeedCanvas, 0, 0, 0,
       &g_sKentec320x240x16_SSD2119, 60, 50, 50, 40,
       CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_OPAQUE, ClrBlack, 0, ClrWhite, g_psFontCmss16b, Speed, 0, 0);

RectangularButton(g_sSpeedSubBttn, 0, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 25, 50, 25, 25,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrBlue, ClrBlue, ClrWhite, ClrWhite,
                   g_psFontCmss16b, "-", 0, 0, 0, 0, onSpeedChange);

RectangularButton(g_sSpeedAddBttn, 0, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 125, 50, 25, 25,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrBlue, ClrBlue, ClrWhite, ClrWhite,
                   g_psFontCmss16b, "+", 0, 0, 0, 0, onSpeedChange);

// Current
static char Current[10] = "100 mA";
Canvas(g_sCurrentCanvas, 0, 0, 0,
       &g_sKentec320x240x16_SSD2119, 50, 100, 75, 40,
       CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_OPAQUE, ClrBlack, 0, ClrWhite, g_psFontCmss16b, Current, 0, 0);

RectangularButton(g_sCurrentSubBttn, 0, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 25, 100, 25, 25,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrBlue, ClrBlue, ClrWhite, ClrWhite,
                   g_psFontCmss16b, "-", 0, 0, 0, 0, onCurrentChange);

RectangularButton(g_sCurrentAddBttn, 0, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 125, 100, 25, 25,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrBlue, ClrBlue, ClrWhite, ClrWhite,
                   g_psFontCmss16b, "+", 0, 0, 0, 0, onCurrentChange);

// Acceleration
static char Acceleration[10] = "50RPM";
Canvas(g_sAccelCanvas, 0, 0, 0,
       &g_sKentec320x240x16_SSD2119, 50, 150, 75, 40,
       CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_OPAQUE, ClrBlack, 0, ClrWhite, g_psFontCmss16b, Acceleration, 0, 0);

RectangularButton(g_sAccelSubBttn, 0, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 25, 150, 25, 25,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrBlue, ClrBlue, ClrWhite, ClrWhite,
                   g_psFontCmss16b, "-", 0, 0, 0, 0, onAccelChange);

RectangularButton(g_sAccelAddBttn, 0, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 125, 150, 25, 25,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrBlue, ClrBlue, ClrWhite, ClrWhite,
                   g_psFontCmss16b, "+", 0, 0, 0, 0, onAccelChange);



void StartStopBttnPress(tWidget *psWidget)
{
    motorStartStop = !motorStartStop;

    if(motorStartStop)
    {
        //
        // Change the button text to indicate the new function.
        //
        PushButtonTextSet(&g_sStartStopBttn, "Stop");
        PushButtonFillColorSet(&g_sStartStopBttn, ClrRed);

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
        PushButtonFillColorSet(&g_sStartStopBttn, ClrGreen);

        WidgetPaint((tWidget *)&g_sStartStopBttn);
    }
    //Event_post(evtHandle, Event_Id_00); // Motor Stop Start Event
}

/* Handles User Speed Change */
void onSpeedChange(tWidget *psWidget){
    // Lower Speed 5%
    if(psWidget == ((tWidget *)&g_sSpeedSubBttn) && (SPEED_USER_LIMIT != 0)){
        SPEED_USER_LIMIT-= 5;
        usprintf(Speed, "%3d%%", SPEED_USER_LIMIT);
        CanvasTextSet(&g_sSpeedCanvas, Speed);
        WidgetPaint((tWidget *)&g_sSpeedCanvas);

        //Event_post(evtHandle, Event_Id_01); // Speed Change Event
     }
    // Increase Speed 5%
    if((psWidget == (tWidget *)&g_sSpeedAddBttn) && (SPEED_USER_LIMIT < SPEED_LIMIT)){
        SPEED_USER_LIMIT = 5 + SPEED_USER_LIMIT;
        usprintf(Speed, "%3d%%", SPEED_USER_LIMIT);
        CanvasTextSet(&g_sSpeedCanvas, Speed);
        WidgetPaint((tWidget *)&g_sSpeedCanvas);

        //Event_post(evtHandle, Event_Id_01); // Speed Change Event
     }
}
/* Handles Current Limit Change */
void onCurrentChange(tWidget *psWidget){
    /* Lower speed 100mA */
    if(psWidget == ((tWidget *)&g_sCurrentSubBttn) && (CURRENT_USER_LIMIT != 0)){
        CURRENT_USER_LIMIT-= 100;
        usprintf(Current, "%5d mA", CURRENT_USER_LIMIT);
        CanvasTextSet(&g_sCurrentCanvas, Current);
        WidgetPaint((tWidget *)&g_sCurrentCanvas);

        //Event_post(evtHandle, Event_Id_02); // Current limit change event
    }
    /* Increase speed 100mA */
    if(psWidget ==((tWidget *)&g_sCurrentAddBttn) && (CURRENT_USER_LIMIT != CURRENT_LIMIT)){
        CURRENT_USER_LIMIT += 100;
        usprintf(Current, "%5d mA", CURRENT_USER_LIMIT);
        CanvasTextSet(&g_sCurrentCanvas, Current);
        WidgetPaint((tWidget *)&g_sCurrentCanvas);
        //Event_post(evtHandle, Event_Id_02); // Current limit change event
    }
}

void onAccelChange(tWidget *psWidget){
    /* Lower acceleration RPM */
    if(psWidget == ((tWidget *)&g_sAccelSubBttn) && (ACCEL_USER_LIMIT != 0)){
        ACCEL_USER_LIMIT-=50;
        usprintf(Acceleration, "%3dRPM", ACCEL_USER_LIMIT);
        CanvasTextSet(&g_sAccelCanvas, Acceleration);
        GPIO_write(Board_LED0, Board_LED_OFF); // Turn on user LED
        WidgetPaint((tWidget *)&g_sAccelCanvas);
        //Event_post(evtHandle, Event_Id_03); // Acceleration Limit change event
    }
    /* Increase acceleration Accel */
    if(psWidget == ((tWidget *)&g_sAccelAddBttn) && (ACCEL_USER_LIMIT != ACCEL_LIMIT)){
        ACCEL_USER_LIMIT+=50;
        usprintf(Acceleration, "%3dRPM", ACCEL_USER_LIMIT);
        CanvasTextSet(&g_sAccelCanvas, Acceleration);
        GPIO_write(Board_LED0, Board_LED_OFF); // Turn on user LED
        WidgetPaint((tWidget *)&g_sAccelCanvas);
        //Event_post(evtHandle, Event_Id_03); // Acceleration Limit change event
    }
}

// Estop flagged
void eStopFxn(UArg arg0, UArg arg1){
    eStop = !eStop;
       UInt posted;
       for(;;){
           //posted = Event_pend(evtHandle, Event_ID_04, Event_Id_NONE,BIOS_WAIT_FOREVER);
           if(eStop & posted){
               CanvasFillColorSet(&g_sEstopLight, ClrRed);
               StartStopBttnPress(&g_sStartStopBttn); // Show Motor is Switched off
           }

           else if(!eStop & posted){ CanvasFillColorSet(&g_sEstopLight, ClrGreen); }
           WidgetPaint((tWidget *)&g_sEstopLight);
       }
}

void onDayNightChange(){
    if(dayNight == "Day"){
        usprintf(dayNight, "Night");
        GPIO_toggle(Board_LED0);
    }
    else{
        usprintf(dayNight, "Day");
        GPIO_toggle(Board_LED0);

    }
    CanvasTextSet(&g_sDayAlert, dayNight);
    WidgetPaint((tWidget *)&g_sDayAlert);
}

void onTabSwap(){
    tabNo = !tabNo;

    if(tabNo){
        PushButtonTextSet(&g_sSwitcher, "Graph");
        DrawHomeScreen();
    }
    else{
        PushButtonTextSet(&g_sSwitcher, "Home");
        // Render Graph Page
    }
}


/*
 *  ======== heartBeatFxn ========
 *  Toggle the Board_LED0. The Task_sleep is determined by arg0 which
 *  is configured for the heartBeat Task instance.
 */
Void heartBeatFxn(UArg arg0, UArg arg1)
{
    while (1) {
        SysCtlDelay(100);
        getCurrentTime();
        WidgetMessageQueueProcess();

//        TouchScreenIntHandler
    }
}

/* Initialise Time at the start of the program */
void initTime(){

    time_t t = time (NULL);
    timeinfo = localtime ( &t );
    timeinfo->tm_year = 2021;
    timeinfo->tm_mon = 5;
    timeinfo->tm_mday = 28;
}

// Gets the current time and updates display
void getCurrentTime(){
    int currentClockTicks = Clock_getTicks();

    /* Check to see if 1000 clocks ticks have passed */
    if((currentClockTicks - clockTicks) >= 1000){
        timeinfo->tm_sec++;
        clockTicks = currentClockTicks; // update clockticks for next comparison.
    }
    /* complicated time update process */
    if((timeinfo->tm_sec) >= 60){
        timeinfo->tm_sec = 0;
        timeinfo->tm_min++;


        if((timeinfo->tm_min) >= 60){
            timeinfo->tm_min = 0;
            timeinfo->tm_hour++;

            if(timeinfo->tm_hour >= 24){
                timeinfo->tm_hour = 0;
                timeinfo->tm_mday++;

            if (timeinfo->tm_mday > 31 && (timeinfo->tm_mon +1 != (2 || 4 || 6 || 9 || 11))) { timeinfo->tm_mon++;}           //31 day month
                else if ((timeinfo->tm_mday > 30 && (timeinfo->tm_mon +1 == (4 || 6 || 9 || 11)))) { timeinfo->tm_mon++; }    //30 day month
                else if ((timeinfo->tm_mday > 28 && (timeinfo->tm_mon +1 == (2)))) { timeinfo->tm_mon++; }                    //28 day month

                if (timeinfo->tm_mon > 12){ timeinfo->tm_sec++; }
            }
        }
    }
    usprintf(currentTime, "%02d/%02d/%4d %02d:%02d:%02d", timeinfo->tm_mday, timeinfo->tm_mon, timeinfo->tm_year, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    CanvasTextSet(&g_sDate, currentTime);
    WidgetPaint((tWidget *)&g_sDate);


}
void RemoveHomeScreen(){
    //Black Background Canvas
        WidgetRemove((tWidget *)&g_sBackground);

        // Limiter Widgets
        WidgetRemove((tWidget *)&g_sLimitTitle);
        WidgetRemove((tWidget *)&g_sSpeedTitle);
        WidgetRemove((tWidget *)&g_sCurrentTitle);
        WidgetRemove((tWidget *)&g_sAccelTitle);
        WidgetRemove((tWidget *)&g_sSpeedSubBttn);
        WidgetRemove((tWidget *)&g_sSpeedAddBttn);
        WidgetRemove((tWidget *)&g_sSpeedCanvas);
        WidgetRemove((tWidget *)&g_sCurrentSubBttn);
        WidgetRemove((tWidget *)&g_sCurrentAddBttn);
        WidgetRemove((tWidget *)&g_sCurrentCanvas);
        WidgetRemove((tWidget *)&g_sAccelSubBttn);
        WidgetRemove((tWidget *)&g_sAccelAddBttn);
        WidgetRemove((tWidget *)&g_sAccelCanvas);

        // Alerts and Date
        WidgetRemove((tWidget *)&g_sEstopText);
        WidgetRemove((tWidget *)&g_sEstopLight);
        WidgetRemove((tWidget *)&g_sDayAlert);
        WidgetRemove((tWidget *)&g_sDate);
        WidgetRemove((tWidget *)&g_sSwitcher);
}

void DrawHomeScreen(){
    //Black Background Canvas
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sBackground);

    // Limiter Widgets
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sLimitTitle);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sSpeedTitle);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sCurrentTitle);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sAccelTitle);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sSpeedSubBttn);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sSpeedAddBttn);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sSpeedCanvas);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sCurrentSubBttn);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sCurrentAddBttn);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sCurrentCanvas);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sAccelSubBttn);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sAccelAddBttn);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sAccelCanvas);

    // Alerts and Date
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sEstopText);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sEstopLight);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sDayAlert);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sDate);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sSwitcher);

    //GPIO_write(Board_LED0, Board_LED_ON);
    FrameDraw(&sContext, "EGH456 Group 6");
    System_printf("Starting the example\nSystem provider is set to SysMin. "
                  "Halt the target to view any SysMin contents in ROV.\n");
    /* SysMin will only print to the console when you call flush or exit */
    System_flush();
    WidgetPaint(WIDGET_ROOT);
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
    Task_construct(&task0Struct, (Task_FuncPtr)heartBeatFxn, &taskParams, NULL);
    taskParams.stack = &task1Stack;
    Task_construct(&task1Struct, (Task_FuncPtr)eStopFxn, &taskParams, NULL);
    evtHandle = Event_handle(&evtStruct);


    //tContext sContext;
    Kentec320x240x16_SSD2119Init(120000000);
    GrContextInit(&sContext, &g_sKentec320x240x16_SSD2119);
    TouchScreenInit(120000000);
    TouchScreenCallbackSet(WidgetPointerMessage);

    initTime();
    DrawHomeScreen();

    /* Start BIOS */
    BIOS_start();

    return (0);
}
