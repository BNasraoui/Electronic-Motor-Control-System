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

#include "inc/hw_memmap.h"

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <driverlib/gpio.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Queue.h>
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

#include "general.h"
#include "sensors.h"

#ifndef GUI_GRAPH_H_
#define GUI_GRAPH_H_

#define AXIS_X_DATA_POINTS              48 /* Must be divisible into GRAPH_WIDTH */
#define DATA_BUFFER_SIZE                32

#define GRAPH_POS_X                     16
#define GRAPH_POS_Y                     32
#define GRAPH_WIDTH                     288
#define GRAPH_HEIGHT                    112
#define AXIS_X_SPACING (GRAPH_WIDTH/AXIS_X_DATA_POINTS)

tContext sGraphContext;

struct GraphData {
    uint32_t prevDataX;
    uint32_t prevDataY;
    float data[AXIS_X_DATA_POINTS];

    uint32_t density;

    uint32_t y_max;
    uint32_t y_estop;
    uint32_t graphHead;

    float axis_y_scale;
} Graph_RPM;

extern void initGUIGraphs(void);

extern uint32_t getGraphY(float y, float scale);

extern void GraphData_init(struct GraphData *data, uint32_t density, uint32_t estop);

extern void drawDataPoint(struct GraphData* graph, uint32_t dx, uint32_t dy);

extern void drawAllGraphData(struct GraphData *graph);

extern void drawGraphBorder(void);

extern void shiftGraphDataLeft(struct GraphData* graph);

extern void clearGraph(struct GraphData *graph);

extern void drawGraph(struct GraphData *graph);

extern void updateGraph(struct GraphData *graph, float newData);

extern void GUI_Graphing(void);

#endif /* GUI_GRAPH_H_ */
