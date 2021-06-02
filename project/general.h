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

#define SENSOR_TASKSTACKSIZE            1024
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

#define START_MOTOR                     Event_Id_16
#define STOP_MOTOR                      Event_Id_17
#define ESTOP                           Event_Id_18

#define EVENT_GUI_HOME_CLEAR            Event_Id_19
#define EVENT_GUI_GRAPH1_CLEAR          Event_Id_20
#define EVENT_GUI_GRAPH2_CLEAR          Event_Id_21

#define EVENT_GRAPH_SPEED               Event_Id_22

#define DEBUG_MODE                      0
#define WATCHDOG_NOTASKS_CHECKEDIN      0x01
#define WATCHDOG_CHECKIN_SENSOR         0x01
#define WATCHDOG_CHECKIN_MOTOR          0x02
#define WATCHDOG_CHECKIN_GUI            0x04
#define ALLTASKS_CHECKEDIN              0x07
char watchDogCheck;

#define CLOCK_PERIOD_150HZ              6    //6ms = ~150Hz
#define CLOCK_PERIOD_2HZ                500  //500ms = 2Hz
#define CLOCK_PERIOD_1HZ                1000
#define CLOCK_PERIOD_10HZ               100
#define CLOCK_PERIOD_100HZ              1
#define CLOCK_TIMEOUT_MS                10  //ms

uint16_t CURRENT_USER_LIMIT;
uint16_t ACCEL_USER_LIMIT;

// motor conditions
bool motorRunning; // current motor status
bool estopFlag; // E-Stop condition flag for GUI and acceleration limits
double desiredSpeed;
double userSpeed;


/* Tasks */
Task_Struct sensorTaskStruct;
Char sensorTaskStack[SENSOR_TASKSTACKSIZE];

Task_Struct guiTaskStruct;
Char guiTaskStack[GUI_TASKSTACKSIZE];

/* Events */
Event_Handle GU_eventHandle;
Event_Handle sensors_eventHandle;
Event_Handle motor_evtHandle;
Event_Handle gui_event_handle;

#endif /* GENERAL_H_ */
