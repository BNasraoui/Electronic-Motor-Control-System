#ifndef __GUI_H__
#define __GUI_H__

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "utils/ustdlib.h"

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/knl/swi.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Queue.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/hal/Seconds.h>

#include "inc/hw_memmap.h"

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>

#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include <driverlib/pin_map.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>

#include "grlib/grlib.h"
#include "grlib/widget.h"
#include "grlib/canvas.h"
#include "grlib/pushbutton.h"
#include "grlib/slider.h"

#include "drivers/touch.h"
#include "drivers/frame.h"
#include "drivers/kentec320x240x16_ssd2119_spi.h"
#include "drivers/pinout.h"

/* Board Header file */
#include "Board.h"

#define SPEED_LIMIT     5820
#define CURRENT_LIMIT   2000
#define ACCEL_LIMIT     500
#define TASKSTACKSIZE   1024



// Graphics
bool eStop;
bool lights;
bool graphingTab;
tContext sContext;
uint32_t clockTicks;

/* Time Values */
struct tm * timeinfo;

Task_Struct task1Struct;
Char task1Stack[TASKSTACKSIZE];

Event_Struct evtStruct;
Event_Handle gui_event_handle;

//extern void StartStopBttnPress(tWidget *psWidget);
//extern void onSpeedChange(tWidget *psWidget);
//extern void onCurrentChange(tWidget *psWidget);
//extern void onAccelChange(tWidget *psWidget);
extern void DrawHomeScreen();
extern void DrawGraphScreen();
extern void RemoveHomeScreen();
extern void RemoveGraphScreen();
extern void onDayNightChange(bool eventType);

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
void getCurrentTime();
void runGUIHomescreen(UInt *events);
//extern void onTabSwap();

extern void initTime();
extern void getCurrentTime();
extern void initTimeStampInterrupt();

extern void initGUIHomescreen(void);

#endif // _GUI_H_
