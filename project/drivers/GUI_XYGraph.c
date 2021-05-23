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

uint16_t getGraphY(struct XYGraphFrame* frame, float y) {
    return (frame->height + frame->pos_y - (y * frame->axis_y_scale));
}

void drawGraphAxisY(struct XYGraphFrame* frame, bool draw) {
    char str[8];
    uint16_t i, j;
    uint16_t scale = 1;

    // For each order of magnitude
    for (i = 0; i < 5; i++) {
        if (frame->y_max < scale*25) {
            // Draw magnitude indicator
            sprintf(str, "%d", scale);

            if (draw) GrContextForegroundSet(&sGraphContext, 0x00787878);
            GrLineDraw(&sGraphContext, frame->pos_x, getGraphY(frame, scale), frame->pos_x + frame->width, getGraphY(frame, scale));
            GrStringDraw(&sGraphContext, str, 8, frame->pos_x + frame->width - 48, getGraphY(frame, scale)-7, 1);

            // Draw notches
            if (draw) GrContextForegroundSet(&sGraphContext, ClrWhite);
            for (j = 0; j < 10 + 1; j++) {
                if (getGraphY(frame, scale * j) > frame->pos_y)
                GrLineDraw(&sGraphContext, frame->pos_x - 2, getGraphY(frame, scale * j), frame->pos_x - 1, getGraphY(frame, scale * j));
            }
        }
        scale = scale * 10;
    }
}

void drawGraphAxis(struct XYGraphFrame* frame) {
    GrLineDraw(&sGraphContext, frame->pos_x, frame->pos_y, frame->pos_x, frame->height + frame->pos_y);
    GrLineDraw(&sGraphContext, frame->pos_x, frame->height + frame->pos_y, frame->width + frame->pos_x, frame->height + frame->pos_y);

    uint16_t i;
    for (i = 0; i < AXIS_X_DATA_POINTS + 1; i++) {
        GrLineDraw(&sGraphContext, frame->pos_x + (frame->axisXSpacing*i), frame->height + frame->pos_y + 1, frame->pos_x + (frame->axisXSpacing*i), frame->height + frame->pos_y + 2);
    }
}

void XYGraph_init_display(struct XYGraphFrame* frame, char* units) {
    GrContextForegroundSet(&sGraphContext, 0x00787878);
    GrStringDraw(&sGraphContext,
             units,
             16,
             frame->pos_x,
             frame->pos_y + frame->height + 8,
             1
    );

    /* Draw Graph Borders */
    drawGraphAxis(frame);
    drawGraphAxisY(frame, true);
}

void drawGraphLag(struct XYGraphFrame* frame, UInt32 time) {
    char str[16];
    sprintf(str, "-%dms", time);

    GrStringDraw(&sGraphContext,
             str,
             16,
             frame->pos_x + 96,
             frame->pos_y + frame->height + 8,
             1
    );
}

void GraphFrame_init(struct XYGraphFrame *frame, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    frame->y_max = 1;
    frame->axis_y_scale = h / frame->y_max;
    frame->pos_x = x;
    frame->pos_y = y;
    frame->width = w;
    frame->height = h;
    frame->axisXSpacing = w/AXIS_X_DATA_POINTS;
}

void GraphData_init(struct XYGraphData *graph, uint16_t density, uint16_t estop) {
    graph->density = density;
    graph->y_estop = estop;
    graph->graphHead = 0;
    // graph->y_max = 1;
}

void drawDataPoint(struct XYGraphFrame* frame, struct XYGraphData* graph, uint16_t dx, uint16_t dy) {
    GrLineDraw(&sGraphContext, graph->prevDataX, getGraphY(frame, graph->prevDataY), dx, getGraphY(frame, dy));
    graph->prevDataX = dx;
    graph->prevDataY = dy;
}

void drawAllGraphData(struct XYGraphFrame* frame, struct XYGraphData *graph) {
    graph->prevDataX = frame->pos_x;
    graph->prevDataY = graph->data[0];

    uint16_t i;
    for (i = 0; i < graph->graphHead; i++) {
        drawDataPoint(frame, graph, frame->pos_x + (i * frame->axisXSpacing), graph->data[i]);
    }
}

void drawCurrentValue(struct XYGraphFrame* frame, struct XYGraphData* graph) {
    char str2[8];
    uint16_t val = graph->data[graph->graphHead - 1];

    sprintf(str2, "%d", val);

    GrLineDraw(&sGraphContext, frame->pos_x - 48, getGraphY(frame, val), frame->pos_x, getGraphY(frame, val));
    GrStringDraw(&sGraphContext, str2, 8, frame->pos_x - 48, getGraphY(frame, val)-7, 1);
}

void clearGraphFrame(struct XYGraphFrame *frame) {
    // GrContextForegroundSet(&sGraphContext, ClrBlack);
}

void clearGraphData(struct XYGraphFrame *frame, struct XYGraphData *graph) {
    GrContextForegroundSet(&sGraphContext, ClrBlack);
    drawAllGraphData(frame, graph);
    drawCurrentValue(frame, graph);
    // drawLogLine(graph, false, 0);

    // e-stop
    GrLineDraw(&sGraphContext, frame->pos_x, getGraphY(frame, graph->y_estop), frame->width + frame->pos_x, getGraphY(frame, graph->y_estop));
}

void drawGraphFrame(struct XYGraphFrame *frame) {
    GrContextForegroundSet(&sGraphContext, ClrWhite);
    drawGraphAxis(frame);
    drawGraphAxisY(frame, true);
}

void drawGraphData(struct XYGraphFrame *frame, struct XYGraphData *graph, uint32_t colour) {
    // e-stop
    GrContextForegroundSet(&sGraphContext, ClrRed);
    GrLineDraw(&sGraphContext, frame->pos_x, getGraphY(frame, graph->y_estop), frame->width + frame->pos_x, getGraphY(frame, graph->y_estop));

    GrContextForegroundSet(&sGraphContext, colour);
    drawAllGraphData(frame, graph);
    drawCurrentValue(frame, graph);
    // drawLogLine(graph, true, value);
}

uint16_t getMax(struct XYGraphData *graph) {
    uint16_t i;
    float m = 0.0F;
    for (i = 0; i < AXIS_X_DATA_POINTS; ++i) {
        if (graph->data[i] > m) m = graph->data[i];
    }
    return m;
}

void updateFrameScale(struct XYGraphFrame *frame) {
    if (frame->maxOnDisplay > frame->y_max || frame->maxOnDisplay < frame->y_max*0.95F) {
        GrContextForegroundSet(&sGraphContext, ClrBlack);
        drawGraphAxisY(frame, false);

        if (frame->maxOnDisplay != 0) {
            frame->axis_y_scale = frame->axis_y_scale * (frame->y_max/frame->maxOnDisplay);
            frame->y_max = frame->maxOnDisplay;
        }
    }
}

void addGraphData(struct XYGraphFrame *frame, struct XYGraphData *graph, uint16_t newData) {
    if (graph->prevDataX == 0 && graph->prevDataY == 0) {
        graph->prevDataX = frame->pos_x + (graph->graphHead * frame->axisXSpacing);
        graph->prevDataY = getGraphY(frame, newData);
    }

    // If graph x axis is full, shift all data back one index
    if (graph->graphHead == AXIS_X_DATA_POINTS) {
        uint16_t i;
        for (i = 0; i < AXIS_X_DATA_POINTS; i++) {
            graph->data[i] = graph->data[i+1];
        }
        --graph->graphHead;
    }

    // Copy data from the sensor buffer, to the graph buffer
    graph->data[graph->graphHead] = newData;

    if (graph->graphHead < AXIS_X_DATA_POINTS) ++graph->graphHead;

    uint16_t max = getMax(graph);
    if (max > frame->maxOnDisplay) frame->maxOnDisplay = max;
}

void updateGraph(struct XYGraphFrame *frame, struct XYGraphData *graph) {
    uint16_t densityAvg = graph->densitySum / graph->density;
    graph->densityCount = 0;
    graph->densitySum = 0;

    addGraphData(frame, graph, densityAvg);
}

bool accumulateGraphData(struct XYGraphData *graph, uint16_t newData) {
    graph->densityCount += 1;
    graph->densitySum += newData;
    if (graph->densityCount >= graph->density) {
        return true;
    }
    return false;
}
