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
    return (frame->zero - (y*frame->axis_y_scale));
}

void drawScaleReference(struct XYGraphFrame* frame, float scale, bool draw) {
    char str[16];
    if (scale < 100) {
        sprintf(str, "%.1f", scale);
    }
    else
    {
        sprintf(str, "%.0f", scale);
    }

    if (draw) GrContextForegroundSet(&sGraphContext, FRAME_COLOUR2);
    uint16_t s = getGraphY(frame, scale);
    if (s > frame->pos_y && s <frame->bottom) {
        GrLineDraw(&sGraphContext, frame->pos_x, getGraphY(frame, scale), frame->pos_x + frame->width, getGraphY(frame, scale));
        GrStringDraw(&sGraphContext, str, 8, frame->pos_x + frame->width - 48, getGraphY(frame, scale)-7, 1);
    }
}

void drawAxisY(struct XYGraphFrame* frame, float scale, bool draw) {
    if (draw) GrContextForegroundSet(&sGraphContext, FRAME_COLOUR);
    uint16_t j, s;
    for (j = 0; j < 10 + 1; j++) {
        s = getGraphY(frame, scale * j);
        if (s > frame->pos_y && s <frame->bottom)
        GrLineDraw(&sGraphContext, frame->pos_x - 3, getGraphY(frame, scale * j), frame->pos_x - 2, getGraphY(frame, scale * j));
    }
}

void drawGraphAxisY(struct XYGraphFrame* frame, bool draw) {
    uint16_t i;
    float scale = 0.1F;

    // For each order of magnitude
    for (i = 0; i < 6; i++) {
        if (frame->y_max < scale*15 && frame->y_max > 0) {
            drawScaleReference(frame, scale, draw);
            drawAxisY(frame, scale, draw);
        }
        scale = scale *-1.0F;
        if (frame->y_min > scale*15 && frame->y_min < 0) {
            drawScaleReference(frame, scale, draw);
            drawAxisY(frame, scale, draw);
        }
        scale = scale * -10.0F;
    }
}

void drawGraphAxis(struct XYGraphFrame* frame) {
    // Y axis
    GrLineDraw(&sGraphContext, frame->pos_x - 1, frame->pos_y, frame->pos_x - 1,frame->bottom);

    uint16_t i;
    for (i = 0; i < AXIS_X_DATA_POINTS + 1; i++) {
        GrLineDraw(&sGraphContext, frame->pos_x + (frame->axisXSpacing*i), frame->bottom + 1, frame->pos_x + (frame->axisXSpacing*i), frame->bottom + 2);
    }
}

void GeneralGraph_init_display(struct XYGraphFrame* frame, char* units) {
    GrContextForegroundSet(&sGraphContext, FRAME_COLOUR2);
    GrStringDraw(&sGraphContext, units, 16, frame->pos_x + (frame->width/2), frame->bottom + 8, 1);

    /* Draw Graph Borders */
    GrContextForegroundSet(&sGraphContext, FRAME_COLOUR);
    drawGraphAxis(frame);
    // drawGraphAxisY(frame, true);
}

void SinglePlotGraph_init_display(struct XYGraphFrame* frame, char* units, char* key) {
    GeneralGraph_init_display(frame, units);

    GrContextForegroundSet(&sGraphContext, LAG_COLOUR);
    GrStringDraw(&sGraphContext, "ms", 8, frame->pos_x + frame->width - GrStringWidthGet(&sGraphContext, "ms", 8),frame->bottom + 8, 1);

    GrContextForegroundSet(&sGraphContext, PLOT_A_COLOUR);
    GrStringDraw(&sGraphContext, key, 8, frame->pos_x, frame->bottom + 8, 1);
}

void TriplePlotGraph_init_display(struct XYGraphFrame* frame, char* units, char* key1, char* key2, char* key3) {
    GeneralGraph_init_display(frame, units);

    GrContextForegroundSet(&sGraphContext, LAG_COLOUR);
    GrStringDraw(&sGraphContext, "ms", 8, frame->pos_x + frame->width - GrStringWidthGet(&sGraphContext, "ms", 8),frame->bottom + 8, 1);
    GrStringDraw(&sGraphContext, "ms", 8, frame->pos_x + frame->width - GrStringWidthGet(&sGraphContext, "ms", 8),frame->bottom + 24, 1);

    uint16_t l1 = GrStringWidthGet(&sGraphContext, key1, 8);
    uint16_t l2 = GrStringWidthGet(&sGraphContext, key2, 8);

    GrContextForegroundSet(&sGraphContext, PLOT_A_COLOUR);
    GrStringDraw(&sGraphContext, key1, 8, frame->pos_x, frame->bottom + 8, 1);
    GrContextForegroundSet(&sGraphContext, PLOT_B_COLOUR);
    GrStringDraw(&sGraphContext, key2, 8, frame->pos_x + l1 + 4, frame->bottom + 8, 1);
    GrContextForegroundSet(&sGraphContext, PLOT_C_COLOUR);
    GrStringDraw(&sGraphContext, key3, 8, frame->pos_x + +l1 + l2 + 8, frame->bottom + 8, 1);
}

void drawGraphLag(struct XYGraphFrame* frame, UInt32 time, uint16_t average) {
    char str[8];
    char stra[8];

    sprintf(str, "-%d", time);
    GrStringDraw(&sGraphContext, str, 8, frame->pos_x + frame->width - 24 - GrStringWidthGet(&sGraphContext, str, 8),frame->bottom + 8, 1);

    if (average > 1) {
        sprintf(stra, "~-%d", time/average);
        GrStringDraw(&sGraphContext, stra, 8, frame->pos_x + frame->width - 24 - GrStringWidthGet(&sGraphContext, stra, 8),frame->bottom + 24, 1);
    }
}

void GraphFrame_init(struct XYGraphFrame *frame, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    frame->y_max = 0;
    frame->y_min = 0;
    frame->axis_y_scale = h;
    frame->pos_x = x;
    frame->pos_y = y;
    frame->width = w;
    frame->height = h;
    frame->bottom = y + h;
    frame->axisXSpacing = w/AXIS_X_DATA_POINTS;
    frame->zero = (h/2) + y;
}

void GraphData_init(struct XYGraphData *graph, uint16_t density, float estop) {
    graph->density = density;
    graph->y_estop = estop;
    graph->graphHead = 0;
}

void drawDataPoint(struct XYGraphFrame* frame, struct XYGraphData* graph, float dx, float dy) {
    GrLineDraw(&sGraphContext, graph->prevDataX, getGraphY(frame, graph->prevDataY), dx, getGraphY(frame, dy));
    graph->prevDataX = dx;
    graph->prevDataY = dy;
}

void drawAllGraphData(struct XYGraphFrame* frame, struct XYGraphData *graph) {
    graph->prevDataX = frame->pos_x;
    graph->prevDataY = graph->data[0];

    uint16_t i;
    for (i = 0; i < graph->graphHead + 1; i++) {
        drawDataPoint(frame, graph, frame->pos_x + (i * frame->axisXSpacing), graph->data[i]);
    }
}

void drawCurrentValue(struct XYGraphFrame* frame, struct XYGraphData* graph, float val) {
    char str2[16];

    if ((getGraphY(frame, graph->data[graph->graphHead - 1]) != getGraphY(frame, graph->data[graph->graphHead]))/* || graph->graphHead - 1 == 0*/) {

        if (val < 100) {
            sprintf(str2, "%.1f", val);
        }
        else
        {
            sprintf(str2, "%.0f", val);
        }

        GrLineDraw(&sGraphContext, frame->pos_x - 12, getGraphY(frame, val), frame->pos_x - 6, getGraphY(frame, val));
        GrStringDraw(&sGraphContext, str2, 8, frame->pos_x - 56, getGraphY(frame, val)-7, 1);
    }
}

void drawZero(struct XYGraphFrame *frame) {
    /* Zero */
    GrLineDraw(&sGraphContext, frame->pos_x - 8, frame->zero, frame->pos_x + frame->width, frame->zero);
}

void clearGraphFrame(struct XYGraphFrame *frame) {
    if (frame->updateFlag) {
        GrContextForegroundSet(&sGraphContext, BACKGROUND_COLOUR);
        drawGraphAxisY(frame, false);
        drawZero(frame);
    }
}

void drawEStopBar(struct XYGraphFrame *frame, struct XYGraphData *graph) {
    uint16_t y = getGraphY(frame, graph->y_estop);
    if (y >= frame->pos_y) {
        // GrLineDraw(&sGraphContext, frame->pos_x, y, frame->width + frame->pos_x, y);
    }
}

void clearGraphData(struct XYGraphFrame *frame, struct XYGraphData *graph) {
    GrContextForegroundSet(&sGraphContext, BACKGROUND_COLOUR);
    if (graph->updateFlag || frame->updateFlag) {
        drawEStopBar(frame, graph);
        drawAllGraphData(frame, graph);
        // drawLogLine(graph, false, 0);
    }

    /* Draw the value of the head of the graph */
    drawCurrentValue(frame, graph, graph->data[graph->graphHead - 1]);
}

void drawGraphFrame(struct XYGraphFrame *frame) {
    GrContextForegroundSet(&sGraphContext, FRAME_COLOUR);
    if (frame->updateFlag) {
        drawGraphAxis(frame);
        drawGraphAxisY(frame, true);
    }
    drawZero(frame);
}

void drawGraphData(struct XYGraphFrame *frame, struct XYGraphData *graph, uint32_t colour) {
    /* Redraw sometimes static things when frame prompted */
    if (frame->updateFlag || graph->updateFlag) {
        GrContextForegroundSet(&sGraphContext, ClrRed);
        drawEStopBar(frame, graph);
        GrContextForegroundSet(&sGraphContext, colour);
        drawAllGraphData(frame, graph);
        // drawLogLine(graph, true, value);
    }

    /* Draw next graph data point to the frame */
    GrContextForegroundSet(&sGraphContext, colour);
    drawDataPoint(frame, graph, frame->pos_x + (graph->graphHead * frame->axisXSpacing), graph->data[graph->graphHead]);


    /* Draw the value of the head of the graph */
    drawCurrentValue(frame, graph, graph->data[graph->graphHead]);

}

float getMax(struct XYGraphData *graph) {
    uint16_t i;
    float m = graph->data[0];
    for (i = 1; i < graph->graphHead + 1; ++i) {
        if (graph->data[i] > m) m = graph->data[i];
    }
    return m;
}

float getMin(struct XYGraphData *graph) {
    uint16_t i;
    float m = graph->data[0];
    for (i = 1; i < graph->graphHead + 1; ++i) {
        if (graph->data[i] < m) m = graph->data[i];
    }
    return m;
}

void updateFrameScale(struct XYGraphFrame *frame) {
    /* Check if the data has exceeded the displayed limits, and if so, flag the frame to update */
    if ((frame->maxOnDisplay > frame->y_max || frame->maxOnDisplay < (frame->y_max/SCALE_UP)*SCALE_DOWN) ||
            (frame->minOnDisplay < frame->y_min) || frame->minOnDisplay > (frame->y_min/SCALE_UP)*SCALE_DOWN) {
        frame->updateFlag = true;
    }
}

void adjustGraph(struct XYGraphFrame *frame, struct XYGraphData *graph) {
    float s, m, z;

    /* reset necessary graph data */
    float newData = graph->data[graph->graphHead];
    if (graph->updateFlag) {
        graph->graphHead = 0;
        graph->data[graph->graphHead] = newData;
        graph->prevDataX = frame->pos_x;
        graph->prevDataY = newData;
    }

    /* adjust y-scale and zero */
    if (frame->updateFlag) {
        /* Set scale so that max axis-y is SCALE_UP larger than that of the largest value,
         * this minimizes re-scaling for minor fluctuations */
        frame->axis_y_scale = ((float)frame->height) / (float) (abs(frame->maxOnDisplay*SCALE_UP) + abs(frame->minOnDisplay*SCALE_UP));
        frame->y_max = frame->maxOnDisplay * SCALE_UP;
        frame->y_min = frame->minOnDisplay * SCALE_UP;

        /* Find where zero would be, given the maximum and minimum values on the graph */
        s = abs(frame->y_max) + abs(frame->y_min);
        /* If all data is only positive */
        if (frame->y_min >= 0 && frame->y_max > frame->y_min) {
            m = (frame->y_min / s) * frame->height;
            z = (frame->pos_y + frame->height) - m;
            frame->zero = z;
        }
        else
        {
            m = (frame->y_max / s) * frame->height;
            z = (frame->pos_y) + m;
            frame->zero = z;
        }
    }
}

void resetFrameBounds(struct XYGraphFrame *frame) {
    /* If all data is only positive */
    if (frame->minOnDisplay >= 0 && frame->maxOnDisplay > frame->minOnDisplay) {
        frame->maxOnDisplay = 1;
        frame->minOnDisplay = 0;
    }
    /* If all data is only negative */
    else if (frame->maxOnDisplay <= 0 && frame->minOnDisplay < frame->maxOnDisplay) {
        frame->maxOnDisplay = 0;
        frame->minOnDisplay = -1;
    }
    /* Both */
    else
    {
        frame->maxOnDisplay = 1;
        frame->minOnDisplay = -1;
    }
}


void addGraphData(struct XYGraphFrame *frame, struct XYGraphData *graph, float newData) {
    /* If previous data points on the frame are undefined, set defaults */
    if (graph->prevDataX == 0 && graph->prevDataY == 0) {
        graph->prevDataX = frame->pos_x;
        graph->prevDataY = newData;
    }

    /* Increment the head of the graph data, and set it to new value */
    ++graph->graphHead;
    graph->data[graph->graphHead] = newData;

    /* If graph head has reached the end of the frame, flag that the displayed graph data needs to be updated */
    if (graph->graphHead == AXIS_X_DATA_POINTS) {
        graph->updateFlag = true;
    }

    /* Get the largest value in this graph */
    float max = getMax(graph);
    if (max > frame->maxOnDisplay) frame->maxOnDisplay = max;

    /* Get the smallest value in this graph */
    float min = getMin(graph);
    if (min < frame->minOnDisplay) frame->minOnDisplay = min;
}

void updateGraph(struct XYGraphFrame *frame, struct XYGraphData *graph, uint16_t density) {
    float densityAvg = graph->densitySum / density;
    graph->densityCount = 0;
    graph->densitySum = 0;

    addGraphData(frame, graph, densityAvg);
}

bool accumulateGraphData(struct XYGraphData *graph, float newData, uint16_t density) {
    /* Accumulate data, minimizing updates per-data received from sensors
     * if sensor transmission rate is too high for the GUI to reasonably keep up with */
    graph->densityCount += 1;
    graph->densitySum += newData;
    if (graph->densityCount >= density) {
        return true;
    }
    return false;
}
