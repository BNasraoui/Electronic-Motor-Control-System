/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_memmap.h"

/* TI-RTOS Header files */
#include <ti/drivers/EMAC.h>
#include <ti/drivers/GPIO.h>

#include "driverlib/gpio.h"

/* Tiva C series macros header files */
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"

/* Board Header file */
#include "Board.h"

/* Created libraries for sub-systems */
#include "general.h"

#include "sensors.h"
#include "driverlib/adc.h"
#include "bmi160.h"

#include "drivers/GUI_graph.h"

void watchDogBite() {
    UInt gateKey;

    if(watchDogCheck == ALLTASKS_CHECKEDIN) {
        Watchdog_clear(watchDogHandle);

        gateKey = GateHwi_enter(gateHwi);
        watchDogCheck = WATCHDOG_NOTASKS_CHECKEDIN;
        GateHwi_leave(gateHwi, gateKey);
    }

    Clock_start(watchDog_ClockHandler);
}

void taskStatusCheck() {
    Event_post(sensors_eventHandle, KICK_DOG);
    Event_post(GU_eventHandle, KICK_DOG);
    //Add event post for motor task
    //Add event post for main GUI if we use it
}

/* Sensor Task Function */
void ReadSensorsFxn() {
    InitI2C_OPT3001();
    InitI2C_BMI160();
    InitADC0_CurrentSense();
    InitADC1_CurrentSense();

    // enable GPIO Hwis for BMI160 and OPT3001
    GPIO_setCallback(Board_BMI160, (GPIO_CallbackFxn)BMI160Fxn);
    GPIO_setCallback(Board_OPT3001, (GPIO_CallbackFxn)OPT3001Fxn);
    GPIO_enableInt(Board_BMI160);
    GPIO_enableInt(Board_OPT3001);

    //Start the timing clocks used to periodically trigger opt3001 and bmi160 reads
    Clock_start(opt3001_ClockHandler);
    Clock_start(adc_ClockHandler);
    Clock_start(watchDog_ClockHandler);

    for(;;) {
        ProcessSensorEvents();
    }
}

/* GUI Task Function */
void GUITaskFxn(void) {
    GUI_Graphing();
}

void InitTasks(void) {
    Task_Params taskParams;
    Task_Params_init(&taskParams);

    /* Sensor Task */
    Task_Params_init(&taskParams);
    taskParams.stackSize = SENSOR_TASKSTACKSIZE;
    taskParams.stack = &task0Stack;
    taskParams.instance->name = "sensorTask";
    taskParams.priority = 2;
    Task_construct(&task0Struct, (Task_FuncPtr) ReadSensorsFxn, &taskParams, NULL);

    /* GUI Task */
    taskParams.stackSize = GUI_TASKSTACKSIZE;
    taskParams.stack = &graphTaskStack;
    taskParams.priority = 1;
    Task_construct(&graphTaskStruct, (Task_FuncPtr) GUITaskFxn, &taskParams, NULL);
}

void InitEvents(void) {
    /* Events */
    Event_Params taskEventParams;
    Event_Params_init(&taskEventParams);

    /* Graph-Update Event */
    GU_eventHandle = Event_create(&taskEventParams, NULL);
    if (GU_eventHandle == NULL) System_abort("GUI event create failed");

    sensors_eventHandle = Event_create(NULL, NULL);
    if(sensors_eventHandle == NULL)  System_abort("Sensors event create failed");
}

/*
 *  ======== main ========
 */
int main(void)
{
    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();
    Board_initI2C();
    Board_initWatchdog();

    InitTasks();
    InitEvents();

    InitSensorDriver();

    /* GUI init */
    initGUIGraphs();
    graphTypeActive = GRAPH_TYPE_CURR;

    watchDogCheck = WATCHDOG_NOTASKS_CHECKEDIN;

    BIOS_start();

    return (0);
}
