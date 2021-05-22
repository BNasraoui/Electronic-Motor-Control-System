/*
 * GUI_XYGraph.h
 *
 *  Created on: 22 May 2021
 *      Author: Mitchell
 */

#ifndef DRIVERS_GUI_XYGRAPH_H_
#define DRIVERS_GUI_XYGRAPH_H_

#define AXIS_X_DATA_POINTS 40

struct XYGraphData {
    uint16_t prevDataX;
    uint16_t prevDataY;
    uint16_t data[AXIS_X_DATA_POINTS];
    float maxOnDisplay;

    uint32_t densitySum;
    uint16_t densityCount;
    uint16_t density;

    uint16_t y_max;
    uint16_t y_estop;
    uint16_t graphHead;

    float axis_y_scale;

    uint16_t pos_x;
    uint16_t pos_y;
    uint16_t width;
    uint16_t height;
    uint16_t axisXSpacing;
} Graph_LUX, Graph_ACCX, Graph_ACCY, Graph_ACCZ;

extern void XYGraph_init_display(struct XYGraphData* graph, char* units);

extern uint16_t getGraphY(struct XYGraphData* graph, float y);

extern void GraphData_init(struct XYGraphData *data, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t density, uint16_t estop);

extern void drawDataPoint(struct XYGraphData* graph, uint16_t dx, uint16_t dy);

extern void drawAllGraphData(struct XYGraphData *graph);

extern void shiftGraphDataLeft(struct XYGraphData* graph);

extern void clearGraph(struct XYGraphData *graph);

extern void drawGraph(struct XYGraphData *graph, float value);

void drawGraphAxisY(struct XYGraphData* graph, bool draw);

extern void drawGraphAxis(struct XYGraphData* graph);

void updateGraph(struct XYGraphData *graph, uint16_t newData);


#endif /* DRIVERS_GUI_XYGRAPH_H_ */
