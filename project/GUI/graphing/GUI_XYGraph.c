/*
 * GUI_XYGraph.c
 *
 *  Created on: 22 May 2021
 *      Author: Mitchell
 */

/* Board Header file */
#include "Board.h"

/* GUI Graphing Header file */
#include "GUI/graphing/GUI_graph.h"
#include "GUI/graphing/GUI_XYGraph.h"
#include "GUI/graphing/GUI_LogGraph.h"
#include "GUI/gui.h"

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

    if (draw) GrContextForegroundSet(&sGUIContext, FRAME_COLOUR2);
    uint16_t s = getGraphY(frame, scale);
    if (s > frame->pos_y && s <frame->bottom) {
        GrLineDraw(&sGUIContext, frame->pos_x, getGraphY(frame, scale), frame->pos_x + frame->width, getGraphY(frame, scale));
        GrStringDraw(&sGUIContext, str, 8, frame->pos_x + frame->width - 48, getGraphY(frame, scale)-7, 1);
    }
}

void drawAxisY(struct XYGraphFrame* frame, float scale, bool draw) {
    if (draw) GrContextForegroundSet(&sGUIContext, FRAME_COLOUR);
    uint16_t j, s;
    for (j = 0; j < 10 + 1; j++) {
        s = getGraphY(frame, scale * j);
        if (s > frame->pos_y && s <frame->bottom)
        GrLineDraw(&sGUIContext, frame->pos_x - 3, getGraphY(frame, scale * j), frame->pos_x - 2, getGraphY(frame, scale * j));
    }
}

void drawGraphAxisY(struct XYGraphFrame* frame, bool draw) {
    uint16_t i;
    float scale = MINIMUM_MAGNITUDE;

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
    GrLineDraw(&sGUIContext, frame->pos_x - 1, frame->pos_y, frame->pos_x - 1,frame->bottom);

    uint16_t i;
    for (i = 0; i < AXIS_X_DATA_POINTS + 1; i++) {
        GrLineDraw(&sGUIContext, frame->pos_x + (frame->axisXSpacing*i), frame->bottom + 1, frame->pos_x + (frame->axisXSpacing*i), frame->bottom + 2);
    }
}

void GeneralGraph_init_display(struct XYGraphFrame* frame, char* units) {
    GrContextForegroundSet(&sGUIContext, FRAME_COLOUR2);
    GrStringDraw(&sGUIContext, units, 16, frame->pos_x + (frame->width/2), frame->bottom + 8, 1);

    /* Draw Graph Borders */
    GrContextForegroundSet(&sGUIContext, FRAME_COLOUR);
    drawLogBar(frame, frame->pos_y + frame->height + 32);
    drawGraphAxis(frame);
    drawGraphAxisY(frame, true);
}

void SinglePlotGraph_init_display(struct XYGraphFrame* frame, char* units, char* key) {
    GeneralGraph_init_display(frame, units);

    GrContextForegroundSet(&sGUIContext, LAG_COLOUR);
    GrStringDraw(&sGUIContext, "ms", 8, frame->pos_x + frame->width - GrStringWidthGet(&sGUIContext, "ms", 8),frame->bottom + 8, 1);

    GrContextForegroundSet(&sGUIContext, PLOT_A_COLOUR);
    GrStringDraw(&sGUIContext, key, 8, frame->pos_x, frame->bottom + 8, 1);
}

void TriplePlotGraph_init_display(struct XYGraphFrame* frame, char* units, char* key1, char* key2, char* key3) {
    GeneralGraph_init_display(frame, units);

    GrContextForegroundSet(&sGUIContext, LAG_COLOUR);
    GrStringDraw(&sGUIContext, "ms", 8, frame->pos_x + frame->width - GrStringWidthGet(&sGUIContext, "ms", 8),frame->bottom + 8, 1);
    // GrStringDraw(&sGUIContext, "ms", 8, frame->pos_x + frame->width - GrStringWidthGet(&sGUIContext, "ms", 8),frame->bottom + 24, 1);

    uint16_t l1 = GrStringWidthGet(&sGUIContext, key1, 8);
    uint16_t l2 = GrStringWidthGet(&sGUIContext, key2, 8);

    GrContextForegroundSet(&sGUIContext, PLOT_A_COLOUR);
    GrStringDraw(&sGUIContext, key1, 8, frame->pos_x, frame->bottom + 8, 1);
    GrContextForegroundSet(&sGUIContext, PLOT_B_COLOUR);
    GrStringDraw(&sGUIContext, key2, 8, frame->pos_x + l1 + 4, frame->bottom + 8, 1);
    GrContextForegroundSet(&sGUIContext, PLOT_C_COLOUR);
    GrStringDraw(&sGUIContext, key3, 8, frame->pos_x + +l1 + l2 + 8, frame->bottom + 8, 1);
}

void drawGraphLag(struct XYGraphFrame* frame, UInt32 time) {
    char str[8];
    //char stra[8];

    sprintf(str, "-%d", time);
    GrStringDraw(&sGUIContext, str, 8, frame->pos_x + frame->width - 24 - GrStringWidthGet(&sGUIContext, str, 8),frame->bottom + 8, 1);

    /*if (average > 1) {
        sprintf(stra, "~-%d", time/average);
        GrStringDraw(&sGUIContext, stra, 8, frame->pos_x + frame->width - 24 - GrStringWidthGet(&sGUIContext, stra, 8),frame->bottom + 24, 1);
    }*/
}

void GraphFrame_init(struct XYGraphFrame *frame, uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool descale_enabled) {
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
    frame->descaleEnabled = descale_enabled;
}

void GraphData_init(struct XYGraphData *graph, float estop) {
    graph->y_estop = estop;
    graph->graphHead = 0;
}

void drawDataPoint(struct XYGraphFrame* frame, struct XYGraphData* graph, float dx, float dy) {
    GrLineDraw(&sGUIContext, graph->prevDataX, getGraphY(frame, graph->prevDataY), dx, getGraphY(frame, dy));
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

float filterData(float data) {
    if (data < 0) {
        return (data + fabs(fmod(data, -MINIMUM_MAGNITUDE)));
    }
    else
    {
        return (data - fmod(data, MINIMUM_MAGNITUDE));
    }
}

void drawCurrentValue(struct XYGraphFrame* frame, struct XYGraphData* graph, float val) {
    GrLineDraw(&sGUIContext, frame->pos_x - 10, getGraphY(frame, val), frame->pos_x - 6, getGraphY(frame, val));
}

void drawZero(struct XYGraphFrame *frame) {
    /* Zero */
    GrLineDraw(&sGUIContext, frame->pos_x - 8, frame->zero, frame->pos_x + frame->width, frame->zero);
}

void clearGraphFrame(struct XYGraphFrame *frame) {
    if (frame->updateFlag) {
        GrContextForegroundSet(&sGUIContext, BACKGROUND_COLOUR);
        drawGraphAxisY(frame, false);
        drawZero(frame);
    }
}

void drawEStopBar(struct XYGraphFrame *frame, struct XYGraphData *graph) {
    uint16_t y = getGraphY(frame, graph->y_estop);
    if (y >= frame->pos_y) {
        // GrLineDraw(&sGUIContext, frame->pos_x, y, frame->width + frame->pos_x, y);
    }
}

void clearGraphData(struct XYGraphFrame *frame, struct XYGraphData *graph) {
    GrContextForegroundSet(&sGUIContext, BACKGROUND_COLOUR);

    if (graph->updateFlag || frame->updateFlag) {
        drawEStopBar(frame, graph);
        drawAllGraphData(frame, graph);
        //drawLogLine(frame, graph, false);
    }

    /* Draw the value of the head of the graph */
    drawCurrentValue(frame, graph, graph->data[graph->graphHead - 1]);
}

void drawGraphFrame(struct XYGraphFrame *frame) {
    GrContextForegroundSet(&sGUIContext, FRAME_COLOUR);
    if (frame->updateFlag) {
        drawGraphAxis(frame);
        drawGraphAxisY(frame, true);
    }
    drawZero(frame);
}

void drawGraphData(struct XYGraphFrame *frame, struct XYGraphData *graph, uint32_t colour) {
    /* Redraw sometimes static things when frame prompted */
    if (frame->updateFlag || graph->updateFlag) {
        GrContextForegroundSet(&sGUIContext, ClrRed);
        drawEStopBar(frame, graph);
        GrContextForegroundSet(&sGUIContext, colour);
        drawAllGraphData(frame, graph);
        //drawLogLine(frame, graph, true);
    }

    /* Draw next graph data point to the frame */
    GrContextForegroundSet(&sGUIContext, colour);
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
    if (((frame->maxOnDisplay > frame->y_max) || (frame->maxOnDisplay < (frame->y_max/SCALE_UP)*SCALE_DOWN)) ||
            ((frame->minOnDisplay < frame->y_min) || (frame->minOnDisplay > (frame->y_min/SCALE_UP)*SCALE_DOWN))
                    ) {
        frame->updateFlag = true;
    }
}

void adjustGraph(struct XYGraphFrame *frame, struct XYGraphData *graph) {
    /* reset necessary graph data */
    float newData = graph->data[graph->graphHead];
    if (graph->updateFlag) {
        graph->graphHead = 0;
        graph->data[graph->graphHead] = newData;
        graph->prevDataX = frame->pos_x;
        graph->prevDataY = newData;
    }
}

void adjustFrame(struct XYGraphFrame *frame) {
    float s, m;

    /* adjust y-scale and zero */
    if (frame->updateFlag) {
        /* Set scale so that max axis-y is SCALE_UP larger than that of the largest value,
         * this minimizes re-scaling for minor fluctuations */
        frame->y_max = frame->maxOnDisplay * SCALE_UP;
        frame->y_min = frame->minOnDisplay * SCALE_UP;
        frame->axis_y_scale = ((float)frame->height) / (fabs(frame->y_max) + fabs(frame->y_min));

        /* Find where zero would be, given the maximum and minimum values on the graph */
        s = fabs(frame->y_max) + fabs(frame->y_min);
        m = (frame->y_min / s) * frame->height;
        frame->zero = (frame->pos_y + frame->height) + m;
    }
}

void resetFrameBounds(struct XYGraphFrame *frame) {
    if (frame->descaleEnabled) {
        frame->maxOnDisplay = 0;
        frame->minOnDisplay = 0;
    }
}

void addDataToGraph(struct XYGraphFrame *frame, struct XYGraphData *graph, float newData) {
    /* Calculate new data from average of sum */
    //float newData = graph->densitySum / density;
    //graph->densityCount = 0;
    //graph->densitySum = 0;
    //newData = filterData(newData);

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
