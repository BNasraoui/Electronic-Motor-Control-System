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
    UInt gateKey, events;

    InitI2C_opt3001();
    InitI2C_BMI160();
    InitADC0_CurrentSense();
    InitADC1_CurrentSense();

    //Clear interrupt bit because
    //uint16_t val;
    //ReadI2C(i2cHandle, OPT3001_SLAVE_ADDRESS, REG_CONFIGURATION, (uint8_t*)&val);

    //enable Hwi for BMI160 and OPT3001
    GPIO_setCallback(Board_BMI160, (GPIO_CallbackFxn)BMI160Fxn);
    GPIO_setCallback(Board_OPT3001, (GPIO_CallbackFxn)OPT3001Fxn);
    GPIO_enableInt(Board_BMI160);
    GPIO_enableInt(Board_OPT3001);
    Clock_start(clockHandler);
    Clock_start(clockHandler2);

    while (1) {
        GPIO_write(Board_LED1, Board_LED_ON);
        events = Event_pend(eventHandler, Event_Id_NONE, (Event_Id_00 + Event_Id_01 + Event_Id_02 + Event_Id_03 + Event_Id_04), BIOS_WAIT_FOREVER);

        if(events & NEW_OPT3001_DATA) {
            GetLuxValue_OPT3001(&rawData);
            Swi_post(swi3Handle);
            Event_post(GU_eventHandle, EVENT_GRAPH_LIGHT);
            System_printf("LUX: %d\n", luxValueFilt.avg);
        }
        if(events & NEW_ACCEL_DATA) {
            GetAccelData_BMI160(&accelX, &accelY, &accelZ);
            Swi_post(swi2Handle);
            System_printf("X: %d\t Y: %d\t Z: %d\n", accelXFilt.avg, accelYFilt.avg, accelZFilt.avg);
        }

        if(events & LOW_HIGH_LIGHT_EVENT) {
            //TURN ON/OFF HEADLIGHTS
            System_printf("LOW/HIGH light even\n");
        }
        if(events & NEW_ADC0_DATA) {
            //Check if limit exceeded, respon accordingly

            //Update display
            System_printf("ADC0: %d\n", ADC0Window.avg);
        }
        if(events & NEW_ADC1_DATA) {
            //Check if limit exceeded, respon accordingly

            //Update display
            System_printf("ADC1: %d\n", ADC1Window.avg);
        }

        System_flush();
        GPIO_write(Board_LED1, Board_LED_OFF);
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
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &sensorTaskStack;
    //taskParams.instance->name = "initI2C_opt3001";
    taskParams.priority = 2;
    Task_construct(&sensorTaskStruct, (Task_FuncPtr) ReadSensorsFxn, &taskParams, NULL);

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
    PinoutSet(false, false);

    initEvents();

    initTasks();

    /* GUI init */
    initGUIGraphs();

    /* Sensor init */
    //This is the custom driver implementation init function
    //That will use Fxn table etc
    //Board_initSensors();
    InitSensorDriver();

    /* Create Hwi Gate Mutex */
    GateHwi_Params_init(&gHwiprms);
    gateHwi = GateHwi_create(&gHwiprms, NULL);
    if (gateHwi == NULL) {
        System_abort("Gate Hwi create failed");
    }

    /* Start BIOS */
    BIOS_start();

    return (0);
}
