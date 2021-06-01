/*
 * gui.c
 *
 *  Created on: 31 May 2021
 *      Author: Mitchell
 */
#include "GUI/graphing/GUI_graph.h"
#include "GUI/graphing/GUI_XYGraph.h"
#include "GUI/homescreen/GUI_homescreen.h"
#include "GUI/graphing/GUI_graphscreen.h"
#include "GUI/gui.h"

void initDisplay(void) {
    Kentec320x240x16_SSD2119Init(SYS_CLK_SPEED);
    GrContextInit(&sGUIContext, &g_sKentec320x240x16_SSD2119);
    TouchScreenInit(SYS_CLK_SPEED);
    TouchScreenCallbackSet(WidgetPointerMessage);
}

void runGUI(void)
{
    UInt graphingEvents, homeEvents;

    initDisplay();
    guiScreen = SCREEN_HOME;

    initGUIGraphs();
    initGUIHomescreen();

    /* GUI */
    while (true) {

        if (guiScreen == SCREEN_GRAPH_DISPLAY) {
            runGUIGraphing(&graphingEvents);
        }
        else if (guiScreen == SCREEN_GRAPH_SELECT) {
            runGUIGraphSelect(&homeEvents);
        }
        else if (guiScreen == SCREEN_HOME)
        {
            runGUIHomescreen(&homeEvents);
        }
    }

    /* END of gui task */
}
