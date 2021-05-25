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

float getGraphY(struct XYGraphFrame* frame, float y) {
    return (frame->height + frame->pos_y - (y * frame->axis_y_scale));
}

void drawGraphAxisY(struct XYGraphFrame* frame, bool draw) {
    char str[16];
    uint16_t i, j;
    float scale = 0.1F;

    // For each order of magnitude
    for (i = 0; i < 6; i++) {
        if (frame->y_max < scale*25) {
            // Draw magnitude indicator
            if (scale < 100) {
                sprintf(str, "%.1f", scale);
            }
            else
            {
                sprintf(str, "%.0f", scale);
            }

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

void GraphData_init(struct XYGraphData *graph, uint16_t density, float estop) {
    graph->density = density;
    graph->y_estop = estop;
    graph->graphHead = 0;
    // graph->y_max = 1;
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

    if (val < 100) {
        sprintf(str2, "%.1f", val);
    }
    else
    {
        sprintf(str2, "%.0f", val);
    }

    GrLineDraw(&sGraphContext, frame->pos_x - 48, getGraphY(frame, val), frame->pos_x - 6, getGraphY(frame, val));
    GrStringDraw(&sGraphContext, str2, 8, frame->pos_x - 48, getGraphY(frame, val)-7, 1);
}

void clearGraphFrame(struct XYGraphFrame *frame) {
    if (frame->updateFlag) {
        GrContextForegroundSet(&sGraphContext, ClrBlack);
        drawGraphAxisY(frame, false);
    }
    // GrContextForegroundSet(&sGraphContext, ClrBlack);
}

void drawEStopBar(struct XYGraphFrame *frame, struct XYGraphData *graph) {
    uint16_t y = getGraphY(frame, graph->y_estop);
    if (y >= frame->pos_y) {
        // GrLineDraw(&sGraphContext, frame->pos_x, y, frame->width + frame->pos_x, y);
    }
}

void clearGraphData(struct XYGraphFrame *frame, struct XYGraphData *graph) {
    GrContextForegroundSet(&sGraphContext, ClrBlack);
    if (graph->updateFlag || frame->updateFlag) {
        drawEStopBar(frame, graph);
        drawAllGraphData(frame, graph);
        // drawLogLine(graph, false, 0);
    }
    if (graph->data[graph->graphHead - 1] != graph->data[graph->graphHead] || graph->graphHead - 1 == 0) drawCurrentValue(frame, graph, graph->data[graph->graphHead - 1]);
}

void drawGraphFrame(struct XYGraphFrame *frame) {
    if (frame->updateFlag) {
        GrContextForegroundSet(&sGraphContext, ClrWhite);
        drawGraphAxis(frame);
        drawGraphAxisY(frame, true);
    }
}

void drawGraphData(struct XYGraphFrame *frame, struct XYGraphData *graph, uint32_t colour) {
    // e-stop
    if (frame->updateFlag || graph->updateFlag) {
        GrContextForegroundSet(&sGraphContext, ClrRed);
        drawEStopBar(frame, graph);
        GrContextForegroundSet(&sGraphContext, colour);
        drawAllGraphData(frame, graph);
        // drawLogLine(graph, true, value);
    }
    else
    {
        GrContextForegroundSet(&sGraphContext, colour);
        drawDataPoint(frame, graph, frame->pos_x + (graph->graphHead * frame->axisXSpacing), graph->data[graph->graphHead]);
    }
    if (graph->data[graph->graphHead - 1] != graph->data[graph->graphHead] || graph->graphHead - 1 == 0) drawCurrentValue(frame, graph, graph->data[graph->graphHead]);

}

float getMax(struct XYGraphData *graph) {
    uint16_t i;
    float m = 0;
    for (i = 0; i < graph->graphHead + 1; ++i) {
        if (graph->data[i] > m) m = graph->data[i];
    }
    return m;
}

void updateFrameScale(struct XYGraphFrame *frame) {
    if (frame->maxOnDisplay > frame->y_max || frame->maxOnDisplay < (frame->y_max/1.2F)*0.9F) {
        frame->updateFlag = true;
    }
}

void adjustGraph(struct XYGraphFrame *frame, struct XYGraphData *graph) {
    float newData = graph->data[graph->graphHead];
    if (graph->updateFlag) {
        graph->graphHead = 0;
        graph->data[graph->graphHead] = newData;
        graph->prevDataX = frame->pos_x;
        graph->prevDataY = newData;
    }

    if ((frame->updateFlag) && (frame->maxOnDisplay != 0)) {
        frame->axis_y_scale = frame->axis_y_scale * (frame->y_max/(frame->maxOnDisplay*1.2F));
        frame->y_max = frame->maxOnDisplay * 1.2F;
    }
}

void addGraphData(struct XYGraphFrame *frame, struct XYGraphData *graph, float newData) {

    if (graph->prevDataX == 0 && graph->prevDataY == 0) {
        graph->prevDataX = frame->pos_x;
        graph->prevDataY = getGraphY(frame, newData);
    }

    ++graph->graphHead;
    graph->data[graph->graphHead] = newData;

    // Copy data from the sensor buffer, to the graph buffer
    if (graph->graphHead == AXIS_X_DATA_POINTS) {
        graph->updateFlag = true;
    }

    float max = getMax(graph);
    if (max > frame->maxOnDisplay) frame->maxOnDisplay = max;
}

void updateGraph(struct XYGraphFrame *frame, struct XYGraphData *graph) {
    float densityAvg = graph->densitySum / graph->density;
    graph->densityCount = 0;
    graph->densitySum = 0;

    addGraphData(frame, graph, densityAvg);


}

bool accumulateGraphData(struct XYGraphData *graph, float newData) {
    graph->densityCount += 1;
    graph->densitySum += newData * -1.0F;
    if (graph->densityCount >= graph->density) {
        return true;
    }
    return false;
}
