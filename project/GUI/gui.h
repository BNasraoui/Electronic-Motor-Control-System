/*
 * gui.h
 *
 *  Created on: 31 May 2021
 *      Author: Mitchell
 */

#ifndef GUI_GUI_H_
#define GUI_GUI_H_

#include <ti/sysbios/knl/Clock.h>

tContext sGUIContext;

Clock_Handle widgetQueue_ClockHandler;

extern void runGUI(void);
extern void UpdateWidgetQueue();
extern void InitGUIDriver();

#endif /* GUI_GUI_H_ */
