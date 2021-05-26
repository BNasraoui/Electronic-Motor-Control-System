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
#include "sensors.h"
#include "drivers/GUI_graph.h"
#include "drivers/GUI_XYGraph.h"

void initGUIGraphs(void) {
    Kentec320x240x16_SSD2119Init(SYS_CLK_SPEED);
    GrContextInit(&sGraphContext, &g_sKentec320x240x16_SSD2119);
    TouchScreenInit(SYS_CLK_SPEED);
    TouchScreenCallbackSet(WidgetPointerMessage);

    GraphFrame_init(&GraphBorder, 64, 32, 240, 112);

    GraphData_init(&Graph_LUX, 600);

    GraphData_init(&Graph_ACCX, 5000);
    GraphData_init(&Graph_ACCY, 8000);
    GraphData_init(&Graph_ACCZ, 9000);
}

void graphLag(struct XYGraphFrame* frame, uint16_t average) {
    static UInt32 last = 0;

    graphLagEnd = Clock_getTicks();
    Uint32 oldT = graphLagTotal;
    graphLagTotal = (double) (graphLagEnd - graphLagStart);
    graphLagStart = 0;

    if (graphLagTotal != last) {
        GrContextForegroundSet(&sGraphContext, BACKGROUND_COLOUR);
        drawGraphLag(frame, oldT, average);

        GrContextForegroundSet(&sGraphContext, LAG_COLOUR);
        drawGraphLag(frame, graphLagTotal, average);

        last = graphLagTotal;
    }
}

void drawSinglePlot(struct XYGraphFrame* frame, struct XYGraphData* graph, float data) {
    if (accumulateGraphData(graph, data, SINGLE_PLOT_DENSITY)) {

        addDataToGraph(frame, graph, SINGLE_PLOT_DENSITY);
        updateFrameScale(frame);

        clearGraphFrame(frame);
        clearGraphData(frame, graph);

        adjustGraph(frame, graph);
        resetFrameBounds(frame);

        drawGraphFrame(frame);
        drawGraphData(frame, graph, PLOT_A_COLOUR);

        frame->updateFlag = false;
        graph->updateFlag = false;

        graphLag(frame, SINGLE_PLOT_DENSITY);
    }
}

void drawTriplePlot(struct XYGraphFrame* frame, struct XYGraphData* graph1, struct XYGraphData* graph2, struct XYGraphData* graph3, float data1, float data2, float data3) {
    static uint16_t skipper = 0;

    bool ready1 = accumulateGraphData(graph1, data1, TRIPLE_PLOT_DENSITY);
    bool ready2 = accumulateGraphData(graph2, data2, TRIPLE_PLOT_DENSITY);
    bool ready3 = accumulateGraphData(graph3, data3, TRIPLE_PLOT_DENSITY);

    if (ready1 && ready2 && ready3) {
        frame->updateFlag = false;
        graph1->updateFlag = false;
        graph2->updateFlag = false;
        graph3->updateFlag = false;

        addDataToGraph(frame, graph1, TRIPLE_PLOT_DENSITY);
        addDataToGraph(frame, graph2, TRIPLE_PLOT_DENSITY);
        addDataToGraph(frame, graph3, TRIPLE_PLOT_DENSITY);
        updateFrameScale(frame);

        skipper = 0;
    }
    else
    {
        switch (skipper) {
            case (1) :
                clearGraphFrame(frame);
                clearGraphData(frame, graph1);
                break;
            case (2) :
                clearGraphData(frame, graph2);
                break;
            case (3) :
                clearGraphData(frame, graph3);
                break;
            case (4) :
                adjustGraph(frame, graph1);
                adjustGraph(frame, graph2);
                adjustGraph(frame, graph3);
                resetFrameBounds(frame);
                break;
            case (5) :
                drawGraphFrame(frame);
                drawGraphData(frame, graph1, PLOT_A_COLOUR);
                break;
            case (6) :
                drawGraphData(frame, graph2, PLOT_B_COLOUR);
                break;
            case (7) :
                drawGraphData(frame, graph3, PLOT_C_COLOUR);
                graphLag(frame, TRIPLE_PLOT_DENSITY);
                break;
        }
    }
    ++skipper;
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
    while (1) {

        events = Event_pend(GU_eventHandle, Event_Id_NONE, (EVENT_GRAPH_LIGHT + EVENT_GRAPH_RPM + EVENT_GRAPH_ACCEL + EVENT_GRAPH_CURR), BIOS_WAIT_FOREVER);

        if (events & EVENT_GRAPH_LIGHT) {
            drawSinglePlot(&GraphBorder, &Graph_LUX, luxValueFilt.avg);
        }

        if (events & EVENT_GRAPH_ACCEL) {
            drawTriplePlot(&GraphBorder, &Graph_ACCX, &Graph_ACCY, &Graph_ACCZ, accelXFilt.G, accelYFilt.G, accelZFilt.G);
        }
    }
}
