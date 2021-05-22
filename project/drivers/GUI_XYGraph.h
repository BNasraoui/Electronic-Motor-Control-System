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
    uint32_t prevDataX;
    uint32_t prevDataY;
    float data[AXIS_X_DATA_POINTS];
    float maxOnDisplay;

    uint32_t density;

    uint32_t y_max;
    uint32_t y_estop;
    uint32_t graphHead;

    float axis_y_scale;

    uint32_t pos_x;
    uint32_t pos_y;
    uint32_t width;
    uint32_t height;
    uint32_t axisXSpacing;
} Graph_LUX, Graph_ACCX, Graph_ACCY, Graph_ACCZ;

extern void XYGraph_init_display(struct XYGraphData* graph, char* units);

extern uint32_t getGraphY(struct XYGraphData* graph, float y);

extern void GraphData_init(struct XYGraphData *data, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t density, uint32_t estop);

extern void drawDataPoint(struct XYGraphData* graph, uint32_t dx, uint32_t dy);

extern void drawAllGraphData(struct XYGraphData *graph);

extern void shiftGraphDataLeft(struct XYGraphData* graph);

extern void clearGraph(struct XYGraphData *graph);

extern void drawGraph(struct XYGraphData *graph, float value);

void drawGraphAxisY(struct XYGraphData* graph, bool draw);

extern void drawGraphAxis(struct XYGraphData* graph);

extern void updateGraph(struct XYGraphData *graph, float newData);


#endif /* DRIVERS_GUI_XYGRAPH_H_ */
