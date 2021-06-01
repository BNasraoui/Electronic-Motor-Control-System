/*
 * gui.c
 *
 *  Created on: 31 May 2021
 *      Author: Mitchell
 */
#include "GUI/graphing/GUI_graph.h"
#include "GUI/graphing/GUI_XYGraph.h"
#include "GUI/homescreen/GUI_homescreen.h"
#include "GUI/gui.h"
#include "general.h"

void initDisplay(void) {
    Kentec320x240x16_SSD2119Init(SYS_CLK_SPEED);
    GrContextInit(&sGUIContext, &g_sKentec320x240x16_SSD2119);
    TouchScreenInit(SYS_CLK_SPEED);
    TouchScreenCallbackSet(WidgetPointerMessage);
}

void UpdateWidgetQueue() {
    WidgetMessageQueueProcess();
}

void InitGUIDriver() {
    Clock_Params clockParams;

    initDisplay();
    initGUIGraphs();
    initGUIHomescreen();

    WidgetMessageQueueProcess();

    Clock_Params_init(&clockParams);
    clockParams.period = CLOCK_PERIOD_100HZ;
    widgetQueue_ClockHandler = Clock_create(UpdateWidgetQueue, CLOCK_TIMEOUT_MS, &clockParams, NULL);
    if (widgetQueue_ClockHandler == NULL) {
     System_abort("watchdog clock create failed");
    }

    Clock_start(widgetQueue_ClockHandler);
}

void runGUI(void)
{
    UInt graphingEvents, homeEvents;

    /* GUI */
    while (true) {
        if (graphingTab) {
            runGUIGraphing(&graphingEvents);
        }
        else
        {
            runGUIHomescreen(&homeEvents);
        }
    }

    /* END of gui task */
}
