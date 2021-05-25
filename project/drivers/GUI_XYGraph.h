/*
 * GUI_XYGraph.h
 *
 *  Created on: 22 May 2021
 *      Author: Mitchell
 */

#ifndef DRIVERS_GUI_XYGRAPH_H_
#define DRIVERS_GUI_XYGRAPH_H_

#define AXIS_X_DATA_POINTS 80

struct XYGraphData {
    uint16_t prevDataX;
    uint16_t prevDataY;
    uint16_t data[AXIS_X_DATA_POINTS];
    // uint16_t y_max;
    uint16_t y_estop;

    uint16_t graphHead;

    uint16_t densitySum;
    uint16_t densityCount;
    uint16_t density;
    bool updateFlag;
} Graph_LUX, Graph_ACCX, Graph_ACCY, Graph_ACCZ;

struct XYGraphFrame {
    float axis_y_scale;
    float maxOnDisplay;
    float y_max;
    uint16_t pos_x;
    uint16_t pos_y;
    uint16_t width;
    uint16_t height;
    uint16_t axisXSpacing;
    bool updateFlag;
} GraphBorder;

extern void XYGraph_init_display(struct XYGraphFrame* frame, char* units);

extern void GraphFrame_init(struct XYGraphFrame *frame, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

extern void GraphData_init(struct XYGraphData *graph, uint16_t density, uint16_t estop);

extern void drawGraphFrame(struct XYGraphFrame *frame);

extern void clearGraphFrame(struct XYGraphFrame *frame);

extern void adjustGraph(struct XYGraphFrame *frame, struct XYGraphData *graph);

extern void updateFrameScale(struct XYGraphFrame *frame);

extern void drawGraphData(struct XYGraphFrame *frame, struct XYGraphData *graph, uint32_t colour);

extern void clearGraphData(struct XYGraphFrame *frame, struct XYGraphData *graph);

extern void updateGraph(struct XYGraphFrame *frame, struct XYGraphData *graph);

extern void drawGraphLag(struct XYGraphFrame* frame, UInt32 time);

extern bool accumulateGraphData(struct XYGraphData *graph, uint16_t newData);


#endif /* DRIVERS_GUI_XYGRAPH_H_ */
