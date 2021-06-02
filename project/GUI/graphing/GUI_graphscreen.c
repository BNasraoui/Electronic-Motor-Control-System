/*
 * GUI_graphscreen.c
 *
 *  Created on: 1 Jun 2021
 *      Author: Mitchell
 */
#include "GUI/graphing/GUI_graph.h"
#include "GUI/graphing/GUI_XYGraph.h"
#include "GUI/homescreen/GUI_homescreen.h"
#include "GUI/graphing/GUI_graphscreen.h"
#include "GUI/gui.h"

bool flagGraph = false;
bool flagHome = false;

void onLightButtonPress(void) {

    flagGraph = true;
    graphTypeActive = GRAPH_TYPE_LIGHT;
    Event_post(gui_event_handle, EVENT_GUI_GRAPH1_CLEAR);
}

void onCurrentButtonPress(void) {

    flagGraph = true;
    graphTypeActive = GRAPH_TYPE_CURR;
    Event_post(gui_event_handle, EVENT_GUI_GRAPH1_CLEAR);
}

void onAccelButtonPress(void) {

    flagGraph = true;
    graphTypeActive = GRAPH_TYPE_ACCELABS;
    Event_post(gui_event_handle, EVENT_GUI_GRAPH1_CLEAR);
}

void onAccelxyzButtonPress(void) {

    flagGraph = true;
    graphTypeActive = GRAPH_TYPE_ACCEL;
    Event_post(gui_event_handle, EVENT_GUI_GRAPH1_CLEAR);
}

void onPowerButtonPress(void) {
    flagGraph = true;
    graphTypeActive = GRAPH_TYPE_POW;
    Event_post(gui_event_handle, EVENT_GUI_GRAPH1_CLEAR);
}

void onSpeedButtonPress(void) {
    flagGraph = true;
    graphTypeActive = GRAPH_TYPE_SPD;
    Event_post(gui_event_handle, EVENT_GUI_GRAPH1_CLEAR);
}

void onHomeButtonPress(void) {

    flagHome = true;

    Event_post(gui_event_handle, EVENT_GUI_GRAPH1_CLEAR);
}

Canvas(g_sGraphBackground, 0, 0, 0,
       &g_sKentec320x240x16_SSD2119, 10, 25, 300, (240 - 25 -10),
       CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);

RectangularButton(g_lightButton, 0, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 25, 25, 200, 25,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrBlue, ClrBlue, ClrWhite, ClrWhite,
                   g_psFontCmss16b, "Light", 0, 0, 0, 0, onLightButtonPress);

RectangularButton(g_currentButton, 0, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 25, 55, 200, 25,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrBlue, ClrBlue, ClrWhite, ClrWhite,
                   g_psFontCmss16b, "Current", 0, 0, 0, 0, onCurrentButtonPress);

RectangularButton(g_accelButton, 0, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 25, 85, 200, 25,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrBlue, ClrBlue, ClrWhite, ClrWhite,
                   g_psFontCmss16b, "Acceleration", 0, 0, 0, 0, onAccelButtonPress);

RectangularButton(g_accelxyzButton, 0, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 25, 115, 200, 25,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrBlue, ClrBlue, ClrWhite, ClrWhite,
                   g_psFontCmss16b, "Accel: x y z", 0, 0, 0, 0, onAccelxyzButtonPress);

RectangularButton(g_powerButton, 0, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 25, 145, 200, 25,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrBlue, ClrBlue, ClrWhite, ClrWhite,
                   g_psFontCmss16b, "Power", 0, 0, 0, 0, onAccelxyzButtonPress);

RectangularButton(g_speedButton, 0, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 25, 175, 200, 25,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrBlue, ClrBlue, ClrWhite, ClrWhite,
                   g_psFontCmss16b, "Speed", 0, 0, 0, 0, onAccelxyzButtonPress);

RectangularButton(g_homeButton, 0, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 190, 205, 100, 25,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrBlue, ClrBlue, ClrWhite, ClrWhite,
                   g_psFontCmss16b, "Config", 0, 0, 0, 0, onHomeButtonPress);

void addGraphscreenWidgets(void) {
    /* Local Widgets */
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sGraphBackground);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_lightButton);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_currentButton);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_accelButton);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_accelxyzButton);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_powerButton);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_speedButton);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_homeButton);

    /* Paint Widgets */
    WidgetPaint(WIDGET_ROOT);
}

void removeGraphscreenWidgets(void) {
    /* Local Widgets */
    WidgetRemove((tWidget *)&g_sGraphBackground);
    WidgetRemove((tWidget *)&g_lightButton);
    WidgetRemove((tWidget *)&g_currentButton);
    WidgetRemove((tWidget *)&g_accelButton);
    WidgetRemove((tWidget *)&g_accelxyzButton);
    WidgetRemove((tWidget *)&g_powerButton);
    WidgetRemove((tWidget *)&g_speedButton);
    WidgetRemove((tWidget *)&g_homeButton);
}

void runGUIGraphSelect(UInt *events) {
    *events = Event_pend(gui_event_handle, Event_Id_NONE, EVENT_GUI_GRAPH1_CLEAR, BIOS_WAIT_FOREVER);

    if (*events & EVENT_GUI_GRAPH1_CLEAR) {
        removeGraphscreenWidgets();

        /* Allows for slow operations to catch up */
        if (flagHome) {
            DrawHomeScreen();
            guiScreen = SCREEN_HOME;
            flagHome = false;
        }
        if (flagGraph) {
            addGraphingWidgets();
            guiScreen = SCREEN_GRAPH_DISPLAY;
            flagGraph = false;
        }
    }
}
