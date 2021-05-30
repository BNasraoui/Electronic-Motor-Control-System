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

#define SENSOR_TASKSTACKSIZE            512
#define GUI_TASKSTACKSIZE               512

#define EVENT_GRAPH_LIGHT               Event_Id_10
#define EVENT_GRAPH_RPM                 Event_Id_11
#define EVENT_GRAPH_ACCEL               Event_Id_12
#define EVENT_GRAPH_CURR                Event_Id_13
#define KICK_DOG                        Event_Id_14

#define DEBUG_MODE                      0
#define WATCHDOG_NOTASKS_CHECKEDIN      0x01
#define WATCHDOG_CHECKIN_SENSOR         0x01
#define WATCHDOG_CHECKIN_MOTOR          0x02
#define WATCHDOG_CHECKIN_GUI            0x04
#define ALLTASKS_CHECKEDIN              0x07
char watchDogCheck;

/* Tasks */
//Task_Struct sensorTaskStruct;
//Char sensorTaskStack[TASKSTACKSIZE];

Task_Struct graphTaskStruct;
Char graphTaskStack[GUI_TASKSTACKSIZE];

/* Events */
// Event_Handle eventHandler;

Event_Handle GU_eventHandle;

#endif /* GENERAL_H_ */
