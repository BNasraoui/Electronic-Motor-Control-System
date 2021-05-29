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

#define SPEED_LIMIT     100
#define CURRENT_LIMIT   2000
#define ACCEL_LIMIT     500
#define TASKSTACKSIZE   1024

#define ESTOP_EVENT     Event_Id_05
#define SPEED_EVENT     Event_Id_06
#define ACCEL_EVENT     Event_Id_07
#define CURRENT_EVENT   Event_Id_08
#define MOTOR_EVENT     Event_Id_09

// Graphics
bool eStop;
bool lights;
tContext sContext;
uint8_t motorStartStop;
uint8_t tabNo;
uint16_t SPEED_USER_LIMIT;
uint16_t CURRENT_USER_LIMIT;
uint16_t ACCEL_USER_LIMIT;
uint32_t clockTicks;

//extern void StartStopBttnPress(tWidget *psWidget);
//extern void onSpeedChange(tWidget *psWidget);
//extern void onCurrentChange(tWidget *psWidget);
//extern void onAccelChange(tWidget *psWidget);
extern void DrawHomeScreen();
//extern void RemoveHomeScreen();
extern void eStopFxn(UArg arg0, UArg arg1);
extern void onDayNightChange(bool eventType);
//extern void onTabSwap();

extern void initTime();
extern void getCurrentTime();

/* Time Values */
struct tm * timeinfo;

Task_Struct task1Struct;
Char task1Stack[TASKSTACKSIZE];

Event_Struct evtStruct;
Event_Handle gui_event_handle;

#endif // _GUI_H_
