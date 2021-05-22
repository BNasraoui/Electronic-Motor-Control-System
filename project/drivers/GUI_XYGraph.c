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
#include "drivers/GUI_XYGraph.h"
#include "drivers/GUI_LogGraph.h"

void XYGraph_init_display(struct XYGraphData* graph, char* units) {
    GrContextForegroundSet(&sGraphContext, 0x00787878);
    GrStringDraw(&sGraphContext,
             units,
             8,
             graph->pos_x,
             graph->pos_y + graph->height + 8,
             1
    );

    /* Draw Graph Borders */
    drawGraphAxis(graph);
    drawGraphAxisY(graph, true);
}

uint32_t getGraphY(struct XYGraphData* graph, float y) {
    return (graph->height + graph->pos_y - (y * graph->axis_y_scale));
}

void GraphData_init(struct XYGraphData *graph, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t density, uint32_t estop) {
    graph->density = density;
    graph->y_estop = estop;
    graph->graphHead = 0;
    graph->y_max = 1;
    graph->axis_y_scale = h / graph->y_max;
    graph->pos_x = x;
    graph->pos_y = y;
    graph->width = w;
    graph->height = h;
    graph->axisXSpacing = w/AXIS_X_DATA_POINTS;
}

void drawDataPoint(struct XYGraphData* graph, uint32_t dx, uint32_t dy) {
    GrLineDraw(&sGraphContext, graph->prevDataX, getGraphY(graph, graph->prevDataY), dx, getGraphY(graph, dy));
    graph->prevDataX = dx;
    graph->prevDataY = dy;
}

void drawAllGraphData(struct XYGraphData *graph) {
    graph->prevDataX = graph->pos_x;
    graph->prevDataY = graph->data[0];

    uint32_t i;
    for (i = 0; i < graph->graphHead; i++) {
        drawDataPoint(graph, graph->pos_x + (i * graph->axisXSpacing), graph->data[i]);
    }
}

void drawCurrentValue(struct XYGraphData* graph) {
    char str2[8];
    uint32_t val = graph->data[graph->graphHead - 1];

    sprintf(&str2, "%d", val);

    GrLineDraw(&sGraphContext, graph->pos_x - 48, getGraphY(graph, val), graph->pos_x, getGraphY(graph, val));
    GrStringDraw(&sGraphContext, str2, 8, graph->pos_x - 48, getGraphY(graph, val)-7, 1);
}

void drawGraphAxisY(struct XYGraphData* graph, bool draw) {
    char str[8];
    uint32_t i, j;
    uint32_t scale = 1;

    for (i = 0; i < 4; i++) {
        if (graph->y_max < scale*75) {
            for (j = 0; j < 10; j++) {
                if (graph->y_max < scale*10 && j == 1) {
                    sprintf(&str, "%d", scale*j);

                    if (draw) GrContextForegroundSet(&sGraphContext, 0x00787878);
                    GrLineDraw(&sGraphContext, graph->pos_x - 48, getGraphY(graph, scale * j), graph->pos_x + graph->width, getGraphY(graph, scale * j));
                    GrStringDraw(&sGraphContext, str, 8, graph->pos_x - 48, getGraphY(graph, scale * j)-7, 1);

                    if (draw) GrContextForegroundSet(&sGraphContext, ClrWhite);
                }
                if (getGraphY(graph, scale * j) > graph->pos_y)
                GrLineDraw(&sGraphContext, graph->pos_x - 2, getGraphY(graph, scale * j), graph->pos_x + 2, getGraphY(graph, scale * j));

            }
        }
        scale = scale * 10;
    }
}

void drawGraphAxis(struct XYGraphData* graph) {
    GrLineDraw(&sGraphContext, graph->pos_x, graph->pos_y, graph->pos_x, graph->height + graph->pos_y);
    GrLineDraw(&sGraphContext, graph->pos_x, graph->height + graph->pos_y, graph->width + graph->pos_x, graph->height + graph->pos_y);

    uint32_t i;
    for (i = 0; i < AXIS_X_DATA_POINTS; i++) {
        GrLineDraw(&sGraphContext, graph->pos_x + (graph->axisXSpacing*i), graph->height + graph->pos_y - 2, graph->pos_x + (graph->axisXSpacing*i), graph->height + graph->pos_y + 2);
    }
}

void shiftGraphDataLeft(struct XYGraphData* graph) {
    uint32_t i;
    for (i = 0; i < AXIS_X_DATA_POINTS; i++) {
        graph->data[i] = graph->data[i+1];
    }
}

void clearGraph(struct XYGraphData *graph) {

    GrContextForegroundSet(&sGraphContext, ClrBlack);
    drawAllGraphData(graph);
    drawCurrentValue(graph);
    drawLogLine(graph, false, 0);

    GrLineDraw(&sGraphContext, graph->pos_x, getGraphY(graph, graph->y_estop), graph->width + graph->pos_x, getGraphY(graph, graph->y_estop));
}

void drawGraph(struct XYGraphData *graph, float value) {
    // Draw the data
    GrContextForegroundSet(&sGraphContext, ClrRed);
    GrLineDraw(&sGraphContext, graph->pos_x, getGraphY(graph, graph->y_estop), graph->width + graph->pos_x, getGraphY(graph, graph->y_estop));

    GrContextForegroundSet(&sGraphContext, ClrWhite);
    drawGraphAxis(graph);
    drawGraphAxisY(graph, true);

    GrContextForegroundSet(&sGraphContext, ClrYellow);
    drawAllGraphData(graph);
    drawCurrentValue(graph);
    drawLogLine(graph, true, value);
}

float getMax(struct XYGraphData *graph) {
    uint32_t i;
    float m = 0.0F;
    for (i = 0; i < AXIS_X_DATA_POINTS; ++i) {
        if (graph->data[i] > m) m = graph->data[i];
    }
    return m;
}

void updateGraph(struct XYGraphData *graph, float newData) {
    clearGraph(graph);

    if (graph->prevDataX == 0 && graph->prevDataY == 0) {
        graph->prevDataX = graph->pos_x + (graph->graphHead * graph->axisXSpacing);
        graph->prevDataY = getGraphY(graph, newData);
    }

    // If graph x axis is full, shift all data back one index
    if (graph->graphHead == AXIS_X_DATA_POINTS) {
        shiftGraphDataLeft(graph);
        --graph->graphHead;
    }

    // Copy data from the sensor buffer, to the graph buffer
    graph->data[graph->graphHead] = newData;
    if (graph->graphHead < AXIS_X_DATA_POINTS) ++graph->graphHead;

    // Re-scale
    graph->maxOnDisplay = getMax(graph);
    if (graph->maxOnDisplay > graph->y_max || graph->maxOnDisplay < graph->y_max*0.95F) {
        GrContextForegroundSet(&sGraphContext, ClrBlack);
        drawGraphAxisY(graph, false);

        if (graph->maxOnDisplay != 0) {
            graph->axis_y_scale = graph->axis_y_scale * (graph->y_max/graph->maxOnDisplay);
            graph->y_max = graph->maxOnDisplay;
        }
    }

    drawGraph(graph, newData);
}
