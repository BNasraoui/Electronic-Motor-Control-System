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

    GraphData_init(&Graph_LUX, 64, 32, 240, 112, 1, 1200);

    GraphData_init(&Graph_ACCX, 64, 32, 240, 112, 3, 50000);
    GraphData_init(&Graph_ACCY, 64, 32, 240, 112, 1, 120);
    GraphData_init(&Graph_ACCZ, 64, 32, 240, 112, 1, 512);
}

void addDataToBuffer(float y) {
    dataBuffer[dataHead] = y;

    ++dataHead;
    if (dataHead > DATA_BUFFER_SIZE) {
        dataHead = 0;
    }
}

void GUI_Graphing(void)
{
    UInt events;

    /* Draw frame */
    FrameDraw(&sGraphContext, "GUI Graphing");

    if (graphTypeActive == GRAPH_TYPE_LIGHT) {
        XYGraph_init_display(&Graph_LUX, "Lux");
    }

    if (graphTypeActive == GRAPH_TYPE_ACCEL) {
        XYGraph_init_display(&Graph_ACCX, "x");
        //XYGraph_init_display(&Graph_ACCY, "x");
        //XYGraph_init_display(&Graph_ACCZ, "x");
    }

    /* forever wait for data */
    while (1) {

        events = Event_pend(GU_eventHandle, Event_Id_NONE, (EVENT_GRAPH_LIGHT + EVENT_GRAPH_RPM + EVENT_GRAPH_ACCEL + EVENT_GRAPH_CURR), BIOS_NO_WAIT);

        if (events & EVENT_GRAPH_LIGHT) {

            //addDataToBuffer((float) luxValueFilt.avg);

            updateGraph(&Graph_LUX, luxValueFilt.avg);

            //dataBuffer[dataTail] = 0;
            //++dataTail;
            //if (dataTail > DATA_BUFFER_SIZE) dataTail = 0;
        }

        if (events & EVENT_GRAPH_ACCEL) {
            // addDataToBuffer((float) luxValueFilt.avg);

            updateGraph(&Graph_ACCX, accelXFilt.avg);
            //updateGraph(&Graph_ACCY, accelYFilt.avg);
            //updateGraph(&Graph_ACCY, accelZFilt.avg);

            /*dataBuffer[dataTail] = 0;
            ++dataTail;
            if (dataTail > DATA_BUFFER_SIZE) dataTail = 0;*/
        }
    }
}
