#include <GUI/homescreen/GUI_homescreen.h>
#include "GUI/graphing/GUI_graph.h"
#include "GUI/gui.h"
#include "sensors/sensors.h"

/*
 *
 *  CREATE WIDGETS
 *
 */

tPushButtonWidget g_sStartStopBttn;
tCanvasWidget     g_sBackground;
tCanvasWidget     g_sGraphBackground;
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
void DrawGraphScreen();
void RemoveHomeScreen();
void RemoveGraphScreen();

void onTabSwap();
void initTime();
//void getCurrentTime();

// The canvas widget acting as the background to the display.
Canvas(g_sBackground, 0, &g_sStartStopBttn, 0,
       &g_sKentec320x240x16_SSD2119, 10, 25, 300, (240 - 25 -10),
       CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);

Canvas(g_sGraphBackground, 0, 0, 0,
       &g_sKentec320x240x16_SSD2119, 10, 25, 300, (240 - 25 -10),
       CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);

CircularButton(g_sStartStopBttn, &g_sBackground, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 275, 50, 25,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrRed, ClrRed, ClrWhite, ClrWhite,
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
                   g_psFontCmss16b, "Graph", 0, 0, 0, 0, onTabSwap); // ADD ONTABSWAP FUNCTION WHEN CONNECTING. NOT CONNECTED YET BECAUSE WILL BORK SYSTEM

// Limit Titles
Canvas(g_sLimitTitle, 0, 0, 0,
       &g_sKentec320x240x16_SSD2119, 60, 20, 50, 40,
       CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_OPAQUE, ClrBlack, 0, ClrWhite, g_psFontCmss16b,"Limits", 0, 0);

Canvas(g_sSpeedTitle, 0, 0, 0,
       &g_sKentec320x240x16_SSD2119, 155, 45, 100, 40,
       CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_OPAQUE, ClrBlack, 0, ClrWhite, g_psFontCmss16b, "Speed (RPM)", 0, 0);
Canvas(g_sCurrentTitle, 0, 0, 0,
       &g_sKentec320x240x16_SSD2119, 155, 95, 70, 40,
       CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_OPAQUE, ClrBlack, 0, ClrWhite, g_psFontCmss16b, "Curr (A)", 0, 0);
Canvas(g_sAccelTitle, 0, 0, 0,
       &g_sKentec320x240x16_SSD2119, 155, 145, 70, 40,
       CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_OPAQUE, ClrBlack, 0, ClrWhite, g_psFontCmss16b, "Accel (G)", 0, 0);

// Speed
static char Speed[10] = "0";
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
static char Current[10] = "500";
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
static char Acceleration[10] = "2";
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
    if(motorRunning)
    {
        /* Change the button text to indicate the new function. */
        PushButtonTextSet(&g_sStartStopBttn, "Stop");
        PushButtonFillColorSet(&g_sStartStopBttn, ClrRed);

        /* Repaint the pushbutton and all widgets beneath it (in this case,
         * the welcome message). */
        WidgetPaint((tWidget *)&g_sStartStopBttn);

       // Event_post(motor_evtHandle, STOP_MOTOR); // Motor Stop Start Event
    }
    else
    {
        /* Change the button text to indicate the new function*/
        PushButtonTextSet(&g_sStartStopBttn, "Start");
        PushButtonFillColorSet(&g_sStartStopBttn, ClrGreen);

        WidgetPaint((tWidget *)&g_sStartStopBttn);

      //  Event_post(motor_evtHandle, START_MOTOR); // Motor Stop Start Event
    }

}

/* Handles User Speed Change */
void onSpeedChange(tWidget *psWidget){
    // Lower Speed 50RPM
    if(psWidget == ((tWidget *)&g_sSpeedSubBttn) && (SPEED_USER_LIMIT > 0)){
        SPEED_USER_LIMIT -= 50;
        usprintf(Speed, "%d", SPEED_USER_LIMIT);
        CanvasTextSet(&g_sSpeedCanvas, Speed);
        WidgetPaint((tWidget *)&g_sSpeedCanvas);
     }
    // Increase Speed 50RPM
    if((psWidget == (tWidget *)&g_sSpeedAddBttn) && (SPEED_USER_LIMIT < SPEED_LIMIT)){
        SPEED_USER_LIMIT += 50;
        usprintf(Speed, "%d", SPEED_USER_LIMIT);
        CanvasTextSet(&g_sSpeedCanvas, Speed);
        WidgetPaint((tWidget *)&g_sSpeedCanvas);
     }
}

/* Handles Current Limit Change */
void onCurrentChange(tWidget *psWidget){
    /* Lower speed 100mA */
    if(psWidget == ((tWidget *)&g_sCurrentSubBttn) && (CURRENT_USER_LIMIT > 0)){
        CURRENT_USER_LIMIT-= 100;
        usprintf(Current, "%d", CURRENT_USER_LIMIT);
        CanvasTextSet(&g_sCurrentCanvas, Current);
        WidgetPaint((tWidget *)&g_sCurrentCanvas);
    }
    /* Increase speed 100mA */
    if(psWidget ==((tWidget *)&g_sCurrentAddBttn) && (CURRENT_USER_LIMIT < CURRENT_LIMIT)){
        CURRENT_USER_LIMIT += 100;
        usprintf(Current, "%d", CURRENT_USER_LIMIT);
        CanvasTextSet(&g_sCurrentCanvas, Current);
        WidgetPaint((tWidget *)&g_sCurrentCanvas);
    }
}

void onAccelChange(tWidget *psWidget){
    /* Lower acceleration RPM */
    if(psWidget == ((tWidget *)&g_sAccelSubBttn) && (ACCEL_USER_LIMIT > 0)){
        ACCEL_USER_LIMIT -= 1;
        usprintf(Acceleration, "%d", ACCEL_USER_LIMIT);
        CanvasTextSet(&g_sAccelCanvas, Acceleration);
        WidgetPaint((tWidget *)&g_sAccelCanvas);
    }
    /* Increase acceleration Accel */
    if(psWidget == ((tWidget *)&g_sAccelAddBttn)  && (ACCEL_USER_LIMIT < ACCEL_LIMIT)){
        ACCEL_USER_LIMIT += 1;
        usprintf(Acceleration, "%d", ACCEL_USER_LIMIT);
        CanvasTextSet(&g_sAccelCanvas, Acceleration);
        WidgetPaint((tWidget *)&g_sAccelCanvas);
    }
}

/* Turns on the night light and updates the display on change */
void onDayNightChange(bool eventType){
    if(eventType == true){
        usprintf(dayNight, "Night");
        GPIO_write(Board_LED1, Board_LED_ON); // Turn on light
        }

    else{
        usprintf(dayNight, "Day");
        GPIO_write(Board_LED1, Board_LED_OFF); // Turn off light
        }

        CanvasTextSet(&g_sDayAlert, dayNight); // Update Widget
        WidgetPaint((tWidget *)&g_sDayAlert);
}

/* Swap between settings and graph tab */
void onTabSwap() {

    if(graphingTab == true) {
        Event_post(GU_eventHandle, EVENT_GUI_SWITCH);
    }
    else if (graphingTab == false) {
        Event_post(gui_event_handle, EVENT_GUI_SWITCH);
    }
}

/*
 *
 *  TIME
 *
 */


void initTimeStampInterrupt(){
    Swi_construct(&swi4Struct, (Swi_FuncPtr)getCurrentTime, &swiParams, NULL);
    swiHandle_TimeStampProc = Swi_handle(&swi4Struct);
    if (swiHandle_TimeStampProc == NULL) {
     System_abort("SWI 4 Timestamp create failed");
    }
}

/* Initialise Time at the start of the program */
void initTime(){

    time_t t = time (NULL);
    timeinfo = localtime ( &t );
    timeinfo->tm_year = 2021;
    timeinfo->tm_mon = 5;
    timeinfo->tm_mday = 28;
    clockTicks = 0;

    usprintf(currentTime, "%02d/%02d/%4d %02d:%02d:%02d", timeinfo->tm_mday, timeinfo->tm_mon, timeinfo->tm_year, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
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

/*
 *
 *  GUI Graphing
 *
 */

/* Draw Graph Screen Widgets */
void DrawGraphScreen() {
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sGraphBackground);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sSwitcher);


    /* Notify that GUI is switching and to stop waiting and event pends */
    // initGraphDrawing();
    WidgetPaint(WIDGET_ROOT);

}

/* Undraw the Graph Widgets */
void RemoveGraphScreen() {
    WidgetRemove((tWidget *) &g_sGraphBackground);
    WidgetRemove((tWidget *) &g_sSwitcher);

    PushButtonTextSet(&g_sSwitcher, "Graph");
    DrawHomeScreen();
}

/*
 *
 *  GUI Homescreen
 *
 */

/* Draw all Home Screen Widgets */
void DrawHomeScreen(){
    //Black Background Canvas
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sBackground);

    // Limiter Widgets
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sStartStopBttn);

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


    /* Draw frame */
    FrameDraw(&sGUIContext, "Configurations");

    WidgetPaint(WIDGET_ROOT);
}

/* Undraw the Home Screen Widgets */
void RemoveHomeScreen() {
    //Black Background Canvas
    WidgetRemove((tWidget *)&g_sBackground);

    // Limiter Widgets
    WidgetRemove((tWidget *)&g_sStartStopBttn);

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

    // Event_post(gui_event_handle, EVENT_GUI_SWITCH);
}

/*
 *
 *  Task
 *
 */

void initGUIHomescreen(void) {
    eStop = false;
    graphingTab = false;
    lights = false;
    motorRunning = 1;
    clockTicks = 0;
    SPEED_USER_LIMIT = 0;
    CURRENT_USER_LIMIT = 500;
    ACCEL_USER_LIMIT = 1;

    initTime();
    DrawHomeScreen();
    initTimeStampInterrupt();
}

void runGUIHomescreen(UInt *events) {

    *events = Event_pend(gui_event_handle, Event_Id_NONE, ESTOP_EVENT + EVENT_GUI_SWITCH, BIOS_WAIT_FOREVER);

    // Estop flagged
    if(*events & ESTOP_EVENT) {
        if(eStop) {
            CanvasFillColorSet(&g_sEstopLight, ClrRed);
            /* Show Motor is Switched off */
            StartStopBttnPress(&g_sStartStopBttn);
        }
        else
        {
            CanvasFillColorSet(&g_sEstopLight, ClrGreen);
        }
        WidgetPaint((tWidget *) &g_sEstopLight);
    }

    if (*events & EVENT_GUI_SWITCH) {
        RemoveHomeScreen();
        PushButtonTextSet(&g_sSwitcher, "Home");
        DrawGraphScreen();
        initGraphDrawing();

        graphingTab = true;
    }
}
