/*
 * GUI_LogGraph.c
 *
 *  Created on: 22 May 2021
 *      Author: Mitchell
 */

#include "GUI_graph.h"
#include "GUI_XYGraph.h"
#include "GUI_LogGraph.h"

/*
void drawLogLine(struct XYGraphData *graph, bool draw, float value) {
    if (draw) GrContextForegroundSet(&sGraphContext, 0x00787878);
    uint32_t i;
    for (i = 0; i < 6; i++) {
        GrLineDraw(&sGraphContext, graph->pos_x + (32*i), (graph->pos_y*2) + graph->height + 3, graph->pos_x + (32*i), (graph->pos_y*2) + graph->height);
    }
    GrLineDraw(&sGraphContext, graph->pos_x, (graph->pos_y*2) + graph->height, graph->pos_x + (32*i), (graph->pos_y*2) + graph->height);
    if (draw) GrContextForegroundSet(&sGraphContext, ClrRed);
    GrLineDraw(&sGraphContext, graph->pos_x + (32*log10(graph->y_estop)), (graph->pos_y*2) + graph->height + 4, graph->pos_x + (32*log10(graph->y_estop)), (graph->pos_y*2) + graph->height + 1);
    GrLineDraw(&sGraphContext, graph->pos_x + (32*log10(graph->y_estop)), (graph->pos_y*2) + graph->height - 4, graph->pos_x + (32*log10(graph->y_estop)), (graph->pos_y*2) + graph->height - 7);

    if (draw) {
        GrContextForegroundSet(&sGraphContext, ClrYellow);
        if (value > 0)
        GrLineDraw(&sGraphContext, graph->pos_x, (graph->pos_y*2) + graph->height - 2, graph->pos_x + (32*log10(value)), (graph->pos_y*2) + graph->height - 2);
    }
    else
    {
        GrLineDraw(&sGraphContext, graph->pos_x, (graph->pos_y*2) + graph->height - 2, graph->width + graph->pos_x, (graph->pos_y*2) + graph->height - 2);
    }
}*/
