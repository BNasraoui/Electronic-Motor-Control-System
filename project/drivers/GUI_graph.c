/*
 * GUI_XYGraph.c
 *
 *  Created on: 22 May 2021
 *      Author: Mitchell
 */

/*
 *  ======== empty.c ========
 */

/* Board Header file */
#include "Board.h"

/* GUI Graphing Header file */
#include "sensors/sensors.h"
#include "sensors/opt3001/opt3001.h"
#include "drivers/GUI_graph.h"
#include "drivers/GUI_XYGraph.h"

void initGUIGraphs(void) {
    Kentec320x240x16_SSD2119Init(SYS_CLK_SPEED);
    GrContextInit(&sGraphContext, &g_sKentec320x240x16_SSD2119);
    TouchScreenInit(SYS_CLK_SPEED);
    TouchScreenCallbackSet(WidgetPointerMessage);

    GraphFrame_init(&GraphBorder, 32, 32, 272, 112);

    GraphData_init(&Graph_LUX, 600);

    GraphData_init(&Graph_ACCX, 5000);
    GraphData_init(&Graph_ACCY, 8000);
    GraphData_init(&Graph_ACCZ, 9000);
}

void graphLag(struct XYGraphFrame* frame) {
    static UInt32 last = 0;

    graphLagEnd = Clock_getTicks();
    //Uint32 oldT = graphLagTotal;
    graphLagTotal = (double) (graphLagEnd - graphLagStart);
    graphLagStart = 0;

    if (graphLagTotal != last) {
        GrContextForegroundSet(&sGraphContext, BACKGROUND_COLOUR);
        drawGraphLag(frame, last);

        GrContextForegroundSet(&sGraphContext, LAG_COLOUR);
        drawGraphLag(frame, graphLagTotal);

        last = graphLagTotal;
    }
}

void drawSinglePlot(struct XYGraphFrame* frame, struct XYGraphData* graph, float data) {
    //if (accumulateGraphData(graph, data, SINGLE_PLOT_DENSITY)) {

        addDataToGraph(frame, graph, data);
        updateFrameScale(frame);

        clearGraphFrame(frame);
        clearGraphData(frame, graph);

        adjustGraph(frame, graph);
        adjustFrame(frame);

        drawGraphFrame(frame);
        drawGraphData(frame, graph, PLOT_A_COLOUR);

        resetFrameBounds(frame);
        frame->updateFlag = false;
        graph->updateFlag = false;

        graphLag(frame);
    //}
}

void drawTriplePlot(struct XYGraphFrame* frame, struct XYGraphData* graph1, struct XYGraphData* graph2, struct XYGraphData* graph3, float data1, float data2, float data3) {

    addDataToGraph(frame, graph1, data1);
    addDataToGraph(frame, graph2, data2);
    addDataToGraph(frame, graph3, data3);
    updateFrameScale(frame);

    clearGraphFrame(frame);
    clearGraphData(frame, graph1);
    clearGraphData(frame, graph2);
    clearGraphData(frame, graph3);

    adjustGraph(frame, graph1);
    adjustGraph(frame, graph2);
    adjustGraph(frame, graph3);
    adjustFrame(frame);

    drawGraphFrame(frame);
    drawGraphData(frame, graph1, PLOT_A_COLOUR);
    drawGraphData(frame, graph2, PLOT_B_COLOUR);
    drawGraphData(frame, graph3, PLOT_C_COLOUR);

    resetFrameBounds(frame);
    frame->updateFlag = false;
    graph1->updateFlag = false;
    graph2->updateFlag = false;
    graph3->updateFlag = false;

    graphLag(frame);
}

void GUI_Graphing(void)
{
    UInt events;

    /* Draw frame */
    FrameDraw(&sGraphContext, "GUI Graphing");


    if (graphTypeActive == GRAPH_TYPE_LIGHT) {
        SinglePlotGraph_init_display(&GraphBorder, "Lux [1:1]", "lux");
    }

    if (graphTypeActive == GRAPH_TYPE_ACCEL) {
        TriplePlotGraph_init_display(&GraphBorder, "G [8:1]", "x", "y", "z");
    }

    /* forever wait for data */
    for(;;) {

        events = Event_pend(GU_eventHandle, Event_Id_NONE, (EVENT_GRAPH_LIGHT + EVENT_GRAPH_RPM + EVENT_GRAPH_ACCEL + EVENT_GRAPH_CURR), BIOS_WAIT_FOREVER);

        if (events & EVENT_GRAPH_LIGHT) {
            drawSinglePlot(&GraphBorder, &Graph_LUX, luxValueFilt.avg);
        }

        if (events & EVENT_GRAPH_ACCEL) {
            drawTriplePlot(&GraphBorder, &Graph_ACCX, &Graph_ACCY, &Graph_ACCZ, accelXFilt.G, accelYFilt.G, accelZFilt.G);
        }
    }
}
