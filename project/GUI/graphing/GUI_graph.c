/*
 * GUI_XYGraph.c
 *
 *  Created on: 22 May 2021
 *      Author: Mitchell
 */

/*
 *
 */
/* GUI Graphing Header file */
#include "sensors/sensors.h"
#include "sensors/opt3001/opt3001.h"
#include "GUI/graphing/GUI_graph.h"
#include "GUI/graphing/GUI_XYGraph.h"
#include "GUI/gui.h"

void initGUIGraphs(void) {

    Kentec320x240x16_SSD2119Init(SYS_CLK_SPEED);
    GrContextInit(&sGraphContext, &g_sKentec320x240x16_SSD2119);
    TouchScreenInit(SYS_CLK_SPEED);
    TouchScreenCallbackSet(WidgetPointerMessage);

    GraphFrame_init(&GraphBorder, 32, 32, 272, 112, false);

    GraphData_init(&Graph_LUX, 600);

    GraphData_init(&Graph_ACCX, 5000);
    GraphData_init(&Graph_ACCY, 8000);
    GraphData_init(&Graph_ACCZ, 9000);

    GraphData_init(&Graph_CURR, 9000);

    /* Default Graph */
    graphTypeActive = GRAPH_TYPE_LIGHT;
    graphingMode = false;
}

void graphLag(struct XYGraphFrame* frame) {
    static UInt32 last = 0;

    graphLagEnd = Clock_getTicks();
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
    addDataToGraph(frame, graph, data);
    updateFrameScale(frame);

    clearGraphFrame(frame);
    drawLogValue(frame, graph, false, frame->pos_y + frame->height + 38);
    clearGraphData(frame, graph);

    adjustGraph(frame, graph);
    adjustFrame(frame);

    drawGraphFrame(frame);
    drawGraphData(frame, graph, PLOT_A_COLOUR);

    GrContextForegroundSet(&sGraphContext, PLOT_A_COLOUR);
    drawLogValue(frame, graph, true, frame->pos_y + frame->height + 38);

    resetFrameBounds(frame);
    frame->updateFlag = false;
    graph->updateFlag = false;

    graphLag(frame);
}

void drawDataValue(struct XYGraphFrame* frame, char* name, float data, uint16_t x, uint16_t y) {
    char str[16];

    sprintf(str, "%s: %.02f", name, data);
    GrStringDraw(&sGraphContext, str, 16, x, y, 1);
}

void drawTriplePlot(struct XYGraphFrame* frame, struct XYGraphData* graph1, struct XYGraphData* graph2, struct XYGraphData* graph3, float data1, float data2, float data3) {

    addDataToGraph(frame, graph1, data1);
    addDataToGraph(frame, graph2, data2);
    addDataToGraph(frame, graph3, data3);
    updateFrameScale(frame);

    clearGraphFrame(frame);

    drawLogValue(frame, graph1, false, frame->pos_y + frame->height + 38);
    drawLogValue(frame, graph2, false, frame->pos_y + frame->height + 40 + 1);
    drawLogValue(frame, graph3, false, frame->pos_y + frame->height + 42 + 2);

    if (DRAW_DATA_TEXT) {
        GrContextForegroundSet(&sGraphContext, BACKGROUND_COLOUR);
        drawDataValue(frame, "x", graph1->prevDataY, 16, frame->bottom + 32);
        drawDataValue(frame, "y", graph2->prevDataY, 96, frame->bottom + 32);
        drawDataValue(frame, "z", graph3->prevDataY, 196, frame->bottom + 32);
    }

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

    if (DRAW_DATA_TEXT) {
        GrContextForegroundSet(&sGraphContext, PLOT_A_COLOUR);
        drawDataValue(frame, "x", data1, 16, frame->bottom + 32);
        GrContextForegroundSet(&sGraphContext, PLOT_B_COLOUR);
        drawDataValue(frame, "y", data2, 96, frame->bottom + 32);
        GrContextForegroundSet(&sGraphContext, PLOT_C_COLOUR);
        drawDataValue(frame, "z", data3, 196, frame->bottom + 32);
    }

    GrContextForegroundSet(&sGraphContext, PLOT_A_COLOUR);
    drawLogValue(frame, graph1, true, frame->pos_y + frame->height + 38);
    GrContextForegroundSet(&sGraphContext, PLOT_B_COLOUR);
    drawLogValue(frame, graph2, true, frame->pos_y + frame->height + 40 + 1);
    GrContextForegroundSet(&sGraphContext, PLOT_C_COLOUR);
    drawLogValue(frame, graph3, true, frame->pos_y + frame->height + 42 + 2);

    resetFrameBounds(frame);
    frame->updateFlag = false;
    graph1->updateFlag = false;
    graph2->updateFlag = false;
    graph3->updateFlag = false;

    graphLag(frame);
}

void initGraphDrawing(void) {
    FrameDraw(&sGraphContext, "GUI Graphing");

    if (graphTypeActive == GRAPH_TYPE_LIGHT) {
        SinglePlotGraph_init_display(&GraphBorder, "Lux [1:1]", "lux");
        GraphBorder.descaleEnabled = true;
    }

    if ((graphTypeActive == GRAPH_TYPE_ACCEL) && tabNo) {
        TriplePlotGraph_init_display(&GraphBorder, "G [8:1]", "x", "y", "z");
        GraphBorder.descaleEnabled = false;
    }

    if (graphTypeActive == GRAPH_TYPE_CURR) {
        SinglePlotGraph_init_display(&GraphBorder, "A [1:1]", "amps");
        GraphBorder.descaleEnabled = true;
    }
}

void GUI_Graphing(void)
{
    UInt events;

    eStop = false;
    tabNo = false;
    lights = false;
    motorStartStop = 1;
    clockTicks = 0;
    SPEED_USER_LIMIT = 5;
    CURRENT_USER_LIMIT = 100;
    ACCEL_USER_LIMIT = 50;

    //StartStopBttnPress(&g_sStartStopBttn); // Show Motor is Switched off

    /* Draw frame */


    /* forever wait for data */
    while (true) {

        if (graphingMode) {

            events = Event_pend(GU_eventHandle, Event_Id_NONE, (EVENT_GRAPH_LIGHT + EVENT_GRAPH_RPM + EVENT_GRAPH_ACCEL + EVENT_GRAPH_CURR), BIOS_NO_WAIT);

            if (events & EVENT_GRAPH_LIGHT) {
                drawSinglePlot(&GraphBorder, &Graph_LUX, luxValueFilt.avg);
            }

            if (events & EVENT_GRAPH_ACCEL) {
                drawTriplePlot(&GraphBorder, &Graph_ACCX, &Graph_ACCY, &Graph_ACCZ, accelXFilt.G, accelYFilt.G, accelZFilt.G);
            }

            if (events & EVENT_GRAPH_CURR) {
                drawSinglePlot(&GraphBorder, &Graph_CURR, ADC1Window.avg);
            }

        }
        else
        {
            eStopFxn();
        }
    }
}
