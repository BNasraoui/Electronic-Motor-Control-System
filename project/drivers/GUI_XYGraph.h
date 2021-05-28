/*
 * GUI_XYGraph.h
 *
 *  Created on: 22 May 2021
 *      Author: Mitchell
 */

#ifndef DRIVERS_GUI_XYGRAPH_H_
#define DRIVERS_GUI_XYGRAPH_H_

#define FRAME_COLOUR ClrWhite
#define FRAME_COLOUR2 ClrDarkGray

#define MINIMUM_MAGNITUDE 0.1

#define AXIS_X_DATA_POINTS 136
#define SCALE_UP 1.2
#define SCALE_DOWN 0.9

struct XYGraphData {
    float prevDataX;
    float prevDataY;
    float data[AXIS_X_DATA_POINTS];
    float y_estop;

    uint16_t graphHead;

    float densitySum;
    uint16_t densityCount;
    bool updateFlag;
} Graph_LUX, Graph_ACCX, Graph_ACCY, Graph_ACCZ, Graph_CURR;

struct XYGraphFrame {
    float axis_y_scale;
    float maxOnDisplay;
    float minOnDisplay;
    float y_max;
    float y_min;
    float zero;
    uint16_t pos_x;
    uint16_t pos_y;
    uint16_t width;
    uint16_t height;
    uint16_t bottom;
    uint16_t axisXSpacing;
    bool updateFlag;
    bool descaleEnabled;
} GraphBorder;

extern void SinglePlotGraph_init_display(struct XYGraphFrame* frame, char* units, char* key);

extern void TriplePlotGraph_init_display(struct XYGraphFrame* frame, char* units, char* key1, char* key2, char* key3);

extern void GraphFrame_init(struct XYGraphFrame *frame, uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool descale_enabled);

extern void GraphData_init(struct XYGraphData *graph, float estop);

extern void drawGraphFrame(struct XYGraphFrame *frame);

extern void clearGraphFrame(struct XYGraphFrame *frame);

extern void adjustGraph(struct XYGraphFrame *frame, struct XYGraphData *graph);

extern void adjustFrame(struct XYGraphFrame *frame);

extern void updateFrameScale(struct XYGraphFrame *frame);

extern void drawGraphData(struct XYGraphFrame *frame, struct XYGraphData *graph, uint32_t colour);

extern void clearGraphData(struct XYGraphFrame *frame, struct XYGraphData *graph);

extern void addDataToGraph(struct XYGraphFrame *frame, struct XYGraphData *graph, float density);

extern void drawGraphLag(struct XYGraphFrame* frame, UInt32 time);

extern void resetFrameBounds(struct XYGraphFrame *frame);

extern bool accumulateGraphData(struct XYGraphData *graph, float newData, uint16_t density);

extern void drawLogBar(struct XYGraphFrame *frame, uint16_t y);

extern void drawLogValue(struct XYGraphFrame *frame, struct XYGraphData *graph, bool draw, uint16_t y);


#endif /* DRIVERS_GUI_XYGRAPH_H_ */
