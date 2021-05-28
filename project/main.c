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

/* Sensor Task Function */
void ReadSensorsFxn() {
    UInt events;

    InitI2C_OPT3001();
    InitI2C_BMI160();
    InitADC0_CurrentSense();
    InitADC1_CurrentSense();

    // re-init i2c in callback mode for periodic sensor reading
    I2C_close(i2cHandle);
    i2cParams.transferMode = I2C_MODE_CALLBACK;
    i2cParams.transferCallbackFxn = I2C_Callback;
    i2cHandle = I2C_open(0, &i2cParams);
    if (i2cHandle == NULL) {
        System_abort("Error Initializing I2C Handle\n");
    }

    // enable Hwi for BMI160 and OPT3001
    GPIO_setCallback(Board_BMI160, (GPIO_CallbackFxn)BMI160Fxn);
    GPIO_setCallback(Board_OPT3001, (GPIO_CallbackFxn)OPT3001Fxn);
    GPIO_enableInt(Board_BMI160);
    GPIO_enableInt(Board_OPT3001);

    //Start the timing clocks used to periodically trigger opt3001 and bmi160 reads
    Clock_start(opt3001_ClockHandler);
    Clock_start(adc_ClockHandler);
    Clock_start(watchDog_ClockHandler);

    while(1) {
        GPIO_write(Board_LED1, Board_LED_ON);
        events = Event_pend(eventHandler, Event_Id_NONE, (Event_Id_00 + Event_Id_01 + Event_Id_02 + Event_Id_03 + Event_Id_04), BIOS_WAIT_FOREVER);

        if(events & NEW_OPT3001_DATA) {
            //GetLuxValue_OPT3001(&rawData);
            //Swi_post(swi3Handle);
            //GPIO_write(Board_LED0, Board_LED_OFF);
            //System_printf("LUX: %f\n", luxValueFilt.avg);

            if (graphTypeActive == GRAPH_TYPE_LIGHT) {
                if (graphLagStart == 0) graphLagStart = Clock_getTicks();
                Event_post(GU_eventHandle, EVENT_GRAPH_LIGHT);
            }

        }
        if(events & NEW_ACCEL_DATA) {
            //GetAccelData_BMI160(&accelX, &accelY, &accelZ);
            //Swi_post(swi2Handle);
            //System_printf("X: %f\t Y: %f\t Z: %f\n", accelXFilt.G, accelYFilt.G, accelZFilt.G);

            if (graphTypeActive == GRAPH_TYPE_ACCEL) {
                if (graphLagStart == 0) graphLagStart = Clock_getTicks();
                Event_post(GU_eventHandle, EVENT_GRAPH_ACCEL);
            }

        }

        if(events & LOW_HIGH_LIGHT_EVENT) {
            //TURN ON/OFF HEADLIGHTS
            //System_printf("LOW/HIGH light even\n");
        }
        if(events & NEW_ADC0_DATA) {
            //Check if limit exceeded, respon accordingly

            //Update display
            //System_printf("ADC0: %f\n", ADC0Window.avg);
        }
        if(events & NEW_ADC1_DATA) {
            //Check if limit exceeded, respon accordingly

            //Update display
            //System_printf("ADC1: %f\n", ADC1Window.avg);
        }
        if(events & KICK_DOG) {
            //System_printf("Setting bit to tell watchdog that this task is ok");
        }

        //System_flush();
        GPIO_write(Board_LED1, Board_LED_OFF);
        Watchdog_clear(watchDogHandle);
    }
}

/* GUI Task Function */
void GUITaskFxn(void) {

    GUI_Graphing();
}

void initTasks(void) {
    /* Tasks */
    Task_Params taskParams;
    Task_Params_init(&taskParams);

    /* Sensor Task */
    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task0Stack;
    taskParams.instance->name = "sensorTask";
    taskParams.priority = 2;
    Task_construct(&task0Struct, (Task_FuncPtr) ReadSensorsFxn, &taskParams, NULL);

    /* GUI Task */
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &graphTaskStack;
    taskParams.priority = 1;
    Task_construct(&graphTaskStruct, (Task_FuncPtr) GUITaskFxn, &taskParams, NULL);
}

void initEvents(void) {
    /* Events */
    Event_Params taskEventParams;
    Event_Params_init(&taskEventParams);

    /* Graph-Update Event */
    GU_eventHandle = Event_create(&taskEventParams, NULL);
    if (GU_eventHandle == NULL) System_abort("Event create failed");
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
    // PinoutSet(false, false);

    initTasks();
    initEvents();
    InitSensorDriver();

    /* GUI init */
    initGUIGraphs();
    graphTypeActive = GRAPH_TYPE_LIGHT;

    /* Start BIOS */
    BIOS_start();

    return (0);
}
