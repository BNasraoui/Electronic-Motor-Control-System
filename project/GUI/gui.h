/*
 * gui.h
 *
 *  Created on: 31 May 2021
 *      Author: Mitchell
 */

#ifndef GUI_GUI_H_
#define GUI_GUI_H_

tContext sGUIContext;

#define SCREEN_HOME 0
#define SCREEN_GRAPH_SELECT 1
#define SCREEN_GRAPH_DISPLAY 2

uint16_t guiScreen;

extern void runGUI(void);

#endif /* GUI_GUI_H_ */
