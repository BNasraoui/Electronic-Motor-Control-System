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

/* Board Header file */
#include "Board.h"

/* GUI Graphing Header file */
#include "drivers/GUI_graph.h"

// Sensor Data Buffer
float dataBuffer[DATA_BUFFER_SIZE];
uint32_t dataHead = 0;
uint32_t dataTail = 0;

uint32_t getGraphY(float y, float scale) {
    return (GRAPH_HEIGHT + GRAPH_POS_Y - (y * scale));
}

void initGUIGraphs(void) {
    Kentec320x240x16_SSD2119Init(SYS_CLK_SPEED);
    GrContextInit(&sGraphContext, &g_sKentec320x240x16_SSD2119);
    TouchScreenInit(SYS_CLK_SPEED);
    TouchScreenCallbackSet(WidgetPointerMessage);

    GraphData_init(&Graph_RPM, 1, 1200);
}

void GraphData_init(struct GraphData *data, uint32_t density, uint32_t estop) {
    data->density = density;
    data->y_estop = estop;
    data->graphHead = 0;
    data->y_max = 1;
    data->axis_y_scale = GRAPH_HEIGHT / data->y_max;
}

void drawDataPoint(struct GraphData* graph, uint32_t dx, uint32_t dy) {
    GrLineDraw(&sGraphContext, graph->prevDataX, getGraphY(graph->prevDataY, graph->axis_y_scale), dx, getGraphY(dy, graph->axis_y_scale));
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
    GrContextForegroundSet(&sGraphContext, ClrWhite);
    GrLineDraw(&sGraphContext, GRAPH_POS_X, GRAPH_POS_Y, GRAPH_POS_X, GRAPH_HEIGHT + GRAPH_POS_Y);
    GrLineDraw(&sGraphContext, GRAPH_POS_X, GRAPH_HEIGHT + GRAPH_POS_Y, GRAPH_WIDTH + GRAPH_POS_X, GRAPH_HEIGHT + GRAPH_POS_Y);

    // x axis notches
    uint32_t i;
    for (i = 0; i < AXIS_X_DATA_POINTS; i++) {
        GrLineDraw(&sGraphContext, GRAPH_POS_X + (AXIS_X_SPACING*i), GRAPH_HEIGHT + GRAPH_POS_Y - 2, GRAPH_POS_X + (AXIS_X_SPACING*i), GRAPH_HEIGHT + GRAPH_POS_Y + 2);
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

    GrContextForegroundSet(&sGraphContext, ClrBlack);
    drawAllGraphData(graph);

    if (newData > graph->y_max) {
        GrLineDraw(&sGraphContext, GRAPH_POS_X, getGraphY(graph->y_estop, graph->axis_y_scale), GRAPH_WIDTH + GRAPH_POS_X, getGraphY(graph->y_estop, graph->axis_y_scale));
    }

    //GrStringDraw(&sGraphContext, dataStr, 32, GRAPH_POS_X + 16, GRAPH_POS_Y + GRAPH_HEIGHT + 16, 1);
    //GrStringDraw(&sGraphContext, dataPeak, 32, GRAPH_POS_X + 16, GRAPH_POS_Y + GRAPH_HEIGHT + 32, 1);
}

void drawGraph(struct GraphData *graph) {
    // Draw the data
    GrContextForegroundSet(&sGraphContext, ClrRed);
    GrLineDraw(&sGraphContext, GRAPH_POS_X, getGraphY(graph->y_estop, graph->axis_y_scale), GRAPH_WIDTH + GRAPH_POS_X, getGraphY(graph->y_estop, graph->axis_y_scale));

    GrContextForegroundSet(&sGraphContext, ClrYellow);
    drawAllGraphData(graph);
    drawGraphBorder();

    /*
        sprintf(&dataStr, "Current: %d", data[graphHead]);
        sprintf(&dataPeak, "Peak: %d", y_max);
        GrStringDraw(&sGraphContext, dataStr, 32, GRAPH_POS_X + 16, GRAPH_POS_Y + GRAPH_HEIGHT + 16, 1);
        GrStringDraw(&sGraphContext, dataPeak, 32, GRAPH_POS_X + 16, GRAPH_POS_Y + GRAPH_HEIGHT + 32, 1);
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

void GUI_Graphing(void)
{
    UInt events;

    /* Draw frame */
    char str[32];
    sprintf(&str, "%dx%d", GRAPH_WIDTH, GRAPH_HEIGHT);
    FrameDraw(&sGraphContext, str);

    /* Draw Graph Borders */
    drawGraphBorder();

    /* forever wait for data */
    while (1) {

        events = Event_pend(GU_eventHandle, Event_Id_NONE, (EVENT_GRAPH_LIGHT + EVENT_GRAPH_RPM + EVENT_GRAPH_ACCEL + EVENT_GRAPH_CURR), BIOS_NO_WAIT);

        // If data is in the buffer, ready to be drawn
        // while (dataHead != dataTail) {
        if (events & EVENT_GRAPH_LIGHT) {
            // addDataToBuffer((float) accelXFilt.avg);
            addDataToBuffer((float) luxValueFilt.avg);

            updateGraph(&Graph_RPM, dataBuffer[dataTail]);

            dataBuffer[dataTail] = 0;

            // Increment sensor buffer tail
            ++dataTail;
            if (dataTail > DATA_BUFFER_SIZE) {
                dataTail = 0;
            }

            // Increase random data range
            // range += 5;
        }

        // addDataToBuffer(rand() % (range));
    }
}
