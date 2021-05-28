#ifndef GUI_H_
#define GUI_H_
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

#define TASKSTACKSIZE   1024
#define SPEED_LIMIT 100 // This is a percent for display.
#define CURRENT_LIMIT 2000 // in milliamps
#define ACCEL_LIMIT 10  // in m/s^2

extern void StartStopBttnPress(tWidget *psWidget);
extern void onSpeedChange(tWidget *psWidget);
extern void onCurrentChange(tWidget *psWidget);
extern void onAccelChange(tWidget *psWidget);
extern void DrawHomeScreen();
extern void RemoveHomeScreen();
extern void eStopFxn(UArg arg0, UArg arg1);
extern void onDayNightChange();
extern void onTabSwap();
extern void initTime();
extern void getCurrentTime();