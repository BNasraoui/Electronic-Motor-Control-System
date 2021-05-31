#ifndef __GUI_GRAPH_H__
#define __GUI_GRAPH_H__
/*
 * GUI_graph.h
 *
 *  Created on: 21 May 2021
 *      Author: Mitchell
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
#include <math.h>
#include <string.h>
#include <time.h>

#include "inc/hw_memmap.h"
#include "inc/hw_memmap.h"

#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Queue.h>
#include <ti/sysbios/hal/Seconds.h>
#include <ti/drivers/GPIO.h>

#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>

#include "grlib/grlib.h"
#include "grlib/widget.h"
#include "grlib/canvas.h"
#include "grlib/pushbutton.h"

#include "drivers/frame.h"
#include "drivers/kentec320x240x16_ssd2119_spi.h"
#include "drivers/pinout.h"
#include "drivers/touch.h"

#include "utils/ustdlib.h"

#include "general.h"

#define DATA_BUFFER_SIZE 32

#define GRAPH_TYPE_LIGHT 1
#define GRAPH_TYPE_ACCEL 2
#define GRAPH_TYPE_CURR 3
#define GRAPH_TYPE_ACCELABS 4

#define SINGLE_PLOT_DENSITY 1
#define TRIPLE_PLOT_DENSITY 8

#define PLOT_A_COLOUR ClrYellow
#define PLOT_B_COLOUR ClrLime
#define PLOT_C_COLOUR ClrLightSkyBlue

#define BACKGROUND_COLOUR ClrBlack
#define LAG_COLOUR ClrMagenta

#define DRAW_DATA_TEXT false

bool graphingMode;
uint32_t graphTypeActive;

/* Clock */
Uint32 graphLagStart, graphLagEnd, graphLagTotal;

extern void initGUIGraphs(void);

extern void initGraphDrawing(void);

extern void runGUIGraphing(UInt *events);

#endif /* GUI_GRAPH_H_ */
