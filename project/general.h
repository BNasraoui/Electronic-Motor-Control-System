/*
 * util.h
 *
 *  Created on: 21 May 2021
 *      Author: Mitchell
 */

#ifndef GENERAL_H_
#define GENERAL_H_

#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Event.h>

#define SYS_CLK_SPEED                   120000000

#define TASKSTACKSIZE                   2048

#define EVENT_GRAPH_LIGHT               Event_Id_01
#define EVENT_GRAPH_RPM                 Event_Id_02
#define EVENT_GRAPH_ACCEL               Event_Id_03
#define EVENT_GRAPH_CURR                Event_Id_03

/* Tasks */
Task_Struct sensorTaskStruct;
Char sensorTaskStack[TASKSTACKSIZE];

Task_Struct graphTaskStruct;
Char graphTaskStack[TASKSTACKSIZE];

/* Events */
Event_Handle eventHandler;

Event_Handle GU_eventHandle;

#endif /* GENERAL_H_ */
