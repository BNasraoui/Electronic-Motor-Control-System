/*
 * GUI_graph.h
 *
 *  Created on: 21 May 2021
 *      Author: Mitchell
 */

#ifndef GUI_GRAPH_H_
#define GUI_GRAPH_H_

#define SYS_CLK_SPEED                   120000000

#define AXIS_X_DATA_POINTS              48 /* Must be divisible into GRAPH_WIDTH */
#define DATA_BUFFER_SIZE                32

#define GRAPH_POS_X                     16
#define GRAPH_POS_Y                     32
#define GRAPH_WIDTH                     288
#define GRAPH_HEIGHT                    112
#define AXIS_X_SPACING (GRAPH_WIDTH/AXIS_X_DATA_POINTS)

tContext sGraphContext;

struct GraphData {
    uint32_t prevDataX;
    uint32_t prevDataY;
    uint32_t data[AXIS_X_DATA_POINTS];

    uint32_t density;

    uint32_t y_max;
    uint32_t y_estop;
    uint32_t graphHead;

    float axis_y_scale;
} Graph_RPM;

extern uint32_t getGraphY(float y, float scale);

extern void GraphData_init(struct GraphData *data, uint32_t density, uint32_t estop);

extern void drawDataPoint(struct GraphData* graph, uint32_t dx, uint32_t dy);

extern void drawAllGraphData(struct GraphData *graph);

extern void drawGraphBorder(void);

extern void shiftGraphDataLeft(struct GraphData* graph);

extern void clearGraph(struct GraphData *graph, float newData);

extern void drawGraph(struct GraphData *graph);

extern void updateGraph(struct GraphData *graph, float newData);

extern void init_Display(void);

extern void GUI_Graphing(UArg arg0, UArg arg1);

#endif /* GUI_GRAPH_H_ */
