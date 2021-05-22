/*
 * GUI_LogGraph.h
 *
 *  Created on: 22 May 2021
 *      Author: Mitchell
 */

#ifndef DRIVERS_GUI_LOGGRAPH_H_
#define DRIVERS_GUI_LOGGRAPH_H_

struct LogGraphData {
    uint32_t y_estop;
    uint32_t pos_x;
    uint32_t pos_y;
    uint32_t width;
    uint32_t height;
} Graph_LogLUX;

void drawLogLine(struct XYGraphData *graph, bool draw, float value);

#endif /* DRIVERS_GUI_LOGGRAPH_H_ */
