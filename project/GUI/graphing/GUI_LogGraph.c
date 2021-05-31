/*
 * GUI_LogGraph.c
 *
 *  Created on: 22 May 2021
 *      Author: Mitchell
 */

#include "GUI/graphing/GUI_graph.h"
#include "GUI/graphing/GUI_XYGraph.h"
#include "GUI/graphing/GUI_LogGraph.h"

void drawLogBar(struct XYGraphFrame *frame, uint16_t y) {
    uint32_t i;
    for (i = 0; i < 5; i++) {
        GrLineDraw(&sGraphContext, frame->pos_x + (64*i), y + 3, frame->pos_x + (64*i), y);
    }

    GrLineDraw(&sGraphContext, frame->pos_x, y, frame->pos_x + (64*(i-1)), y);
    //if (draw) GrContextForegroundSet(&sGraphContext, ClrRed);
    //GrLineDraw(&sGraphContext, frame->pos_x + (32*log10(graph->y_estop)), y + 4, frame->pos_x + (32*log10(graph->y_estop)), y + 1);
    //GrLineDraw(&sGraphContext, frame->pos_x + (32*log10(graph->y_estop)), y - 4, frame->pos_x + (32*log10(graph->y_estop)), y - 7);
}

void drawLogValue(struct XYGraphFrame *frame, struct XYGraphData *graph, bool draw, uint16_t y) {
    float scaleUp = 10.0F;
    float value;
    if (draw) {
        value = graph->data[graph->graphHead] * scaleUp;
    }
    else
    {
        value = graph->data[graph->graphHead - 1] * scaleUp;
    }

    if (graph->data[graph->graphHead] != graph->data[graph->graphHead - 1]) {
        if (!draw) GrContextForegroundSet(&sGraphContext, BACKGROUND_COLOUR);
        if (value > 0.1F * scaleUp) {
            GrLineDraw(&sGraphContext, frame->pos_x, y, frame->pos_x + (64*log10(value)), y);
            GrLineDraw(&sGraphContext, frame->pos_x, y + 1, frame->pos_x + (64*log10(value)), y + 1);
        }
        else if (value < -0.1F * scaleUp)
        {
            GrLineDraw(&sGraphContext, frame->pos_x + (64*3), y, frame->pos_x + (64*3) - (64*log10(fabs(value))), y);
            GrLineDraw(&sGraphContext, frame->pos_x + (64*3), y + 1, frame->pos_x + (64*3) - (64*log10(fabs(value))), y + 1);
        }
    }
}
