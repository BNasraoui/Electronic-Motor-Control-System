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
#define GUI_TASKSTACKSIZE               2048

#define LOW_HIGH_LIGHT_EVENT            Event_Id_00
#define NEW_OPT3001_DATA                Event_Id_01
#define NEW_ACCEL_DATA                  Event_Id_02
#define NEW_ADC0_DATA                   Event_Id_03
#define NEW_ADC1_DATA                   Event_Id_04
#define ESTOP_EVENT                     Event_Id_05
#define SPEED_EVENT                     Event_Id_06
#define ACCEL_EVENT                     Event_Id_07
#define CURRENT_EVENT                   Event_Id_08
#define MOTOR_EVENT                     Event_Id_09
#define EVENT_GRAPH_LIGHT               Event_Id_10
#define EVENT_GRAPH_RPM                 Event_Id_11
#define EVENT_GRAPH_ACCEL               Event_Id_12
#define EVENT_GRAPH_CURR                Event_Id_13
#define KICK_DOG                        Event_Id_14
#define EVENT_GUI_HOME_CLEAR            Event_Id_15
#define EVENT_GUI_GRAPH1_CLEAR          Event_Id_16
#define EVENT_GUI_GRAPH2_CLEAR          Event_Id_17

#define DEBUG_MODE                      0
#define WATCHDOG_NOTASKS_CHECKEDIN      0x01
#define WATCHDOG_CHECKIN_SENSOR         0x01
#define WATCHDOG_CHECKIN_MOTOR          0x02
#define WATCHDOG_CHECKIN_GUI            0x04
#define ALLTASKS_CHECKEDIN              0x07
char watchDogCheck;

uint8_t motorRunning;
uint16_t SPEED_USER_LIMIT;
uint16_t CURRENT_USER_LIMIT;
uint16_t ACCEL_USER_LIMIT;

/* Tasks */
Task_Struct sensorTaskStruct;
Char sensorTaskStack[SENSOR_TASKSTACKSIZE];

Task_Struct guiTaskStruct;
Char guiTaskStack[GUI_TASKSTACKSIZE];

/* Events */
Event_Handle GU_eventHandle;
Event_Handle sensors_eventHandle;
Event_Handle gui_event_handle;

#endif /* GENERAL_H_ */
