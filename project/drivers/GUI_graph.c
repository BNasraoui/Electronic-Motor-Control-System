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

    GraphData_init(&Graph_LUX, GRAPH_LIGHT_DENSITY, 600);

    GraphData_init(&Graph_ACCX, GRAPH_ACCEL_DENSITY, 50000);
    GraphData_init(&Graph_ACCY, GRAPH_ACCEL_DENSITY, 55000);
    GraphData_init(&Graph_ACCZ, GRAPH_ACCEL_DENSITY, 45000);
}

void graphLag(struct XYGraphFrame* frame) {
    graphLagEnd = Clock_getTicks();
    Uint32 oldT = graphLagTotal;
    graphLagTotal = (double) (graphLagEnd - graphLagStart);
    graphLagStart = 0;

    GrContextForegroundSet(&sGraphContext, ClrBlack);
    drawGraphLag(frame, oldT);

    GrContextForegroundSet(&sGraphContext, 0x00787878);
    drawGraphLag(frame, graphLagTotal);
}

void GUI_Graphing(void)
{
    UInt events;
    uint16_t skipper = 0;

    /* Draw frame */
    FrameDraw(&sGraphContext, "GUI Graphing");

    if (graphTypeActive == GRAPH_TYPE_LIGHT) {
        XYGraph_init_display(&GraphBorder, "Lux [1:1]");
    }

    if (graphTypeActive == GRAPH_TYPE_ACCEL) {
        XYGraph_init_display(&GraphBorder, "G [8:1]");
    }

    /* forever wait for data */
    while (1) {

        events = Event_pend(GU_eventHandle, Event_Id_NONE, (EVENT_GRAPH_LIGHT + EVENT_GRAPH_RPM + EVENT_GRAPH_ACCEL + EVENT_GRAPH_CURR), BIOS_NO_WAIT);

        if (events & EVENT_GRAPH_LIGHT) {

            if (accumulateGraphData(&Graph_LUX, luxValueFilt.avg)) {
                clearGraphFrame(&GraphBorder);
                clearGraphData(&GraphBorder, &Graph_LUX);

                updateGraph(&GraphBorder, &Graph_LUX);
                updateFrameScale(&GraphBorder);

                drawGraphFrame(&GraphBorder);
                drawGraphData(&GraphBorder, &Graph_LUX, ClrYellow);

                GraphBorder.maxOnDisplay = 0;

                graphLag(&GraphBorder);
            }
        }

        if (events & EVENT_GRAPH_ACCEL) {

            bool xready = accumulateGraphData(&Graph_ACCX, accelXFilt.avg);
            bool yready = accumulateGraphData(&Graph_ACCY, accelYFilt.avg);
            bool zready = accumulateGraphData(&Graph_ACCZ, accelZFilt.avg);

            if (!(xready && yready && zready)) {
                /* If the data is still being accumulated by the graphs, draw the last set of data */
                switch (skipper) {
                    case (0) :
                        updateGraph(&GraphBorder, &Graph_ACCX);
                        drawGraphData(&GraphBorder, &Graph_ACCX, ClrYellow);
                        break;
                    case (1) :
                        updateGraph(&GraphBorder, &Graph_ACCY);
                        drawGraphData(&GraphBorder, &Graph_ACCY, ClrLime);
                        break;
                    case (2) :
                        updateGraph(&GraphBorder, &Graph_ACCZ);
                        drawGraphData(&GraphBorder, &Graph_ACCZ, ClrLightSkyBlue);
                        break;
                    case (GRAPH_ACCEL_DENSITY-3) :
                        clearGraphFrame(&GraphBorder);
                        break;
                    case (GRAPH_ACCEL_DENSITY-2) :
                        clearGraphData(&GraphBorder, &Graph_ACCX);
                        clearGraphData(&GraphBorder, &Graph_ACCY);
                        break;
                    case (GRAPH_ACCEL_DENSITY-1) :
                        clearGraphData(&GraphBorder, &Graph_ACCZ);
                        updateFrameScale(&GraphBorder);
                        drawGraphFrame(&GraphBorder);

                        graphLag(&GraphBorder);

                        break;
                }
            }

            ++skipper;
            if (skipper > 7) {
                skipper = 0;
                GraphBorder.maxOnDisplay = 0;
            }

        }
    }
}
