#ifndef __MOTOR_H__
#define __MOTOR_H__


/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Types.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/event.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/knl/Swi.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>

/* Board Header file */
#include "Board.h"
#include <motorlib.h>
#include <xdc/runtime/Error.h>

/* other includes */
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"
#include <math.h>

#define TASKSTACKSIZE       512

#define CLOCK_PERIOD_10Hz   100 // 100ms = 10Hz -> Speed Calculation
#define CLOCK_PERIOD_100Hz  10  // 10ms = 100Hz -> Speed filtering
#define CLOCK_PERIOD_50Hz   1  // 20ms = 50Hz -> PID loop was 20
#define CLOCK_PERIOD_500Hz  2   // 2ms = 500Hz -> acceleration limit
#define CLOCK_TIMEOUT_MS    10  // 10ms

#define WINDOW_SIZE         5   // filter window size

#define PID_TAU             0.1 // filter constant
#define PID_PERIOD          0.001 // 20ms or 50Hz

#define PID_MIN_SPEED       0
#define PID_MAX_SPEED       5820 // no load speed at max duty cycle (measured)

#define PID_LIM_MIN_INT     -5.0
#define PID_LIM_MAX_INT     5.0


typedef struct {

    // controller gains
    double Kp;
    double Ki;
    double Kd;

    // Derivative low pass-filter time constant
    double tau;

    // output limits
    double maxSpeed;
    double minSpeed;

    // Integrator limits
    float limMinInt;
    float limMaxInt;

    // sample time in secs
    double T;

    // controller memory
    double integrator;
    double previousError;
    double differentiator;
    double previousSpeed;

    // controller output
    double outputSpeed;

} PIDcontroller;


typedef struct Sliding_Window5{
    uint8_t index;
    double sum;
    double avg;
    double data[WINDOW_SIZE];
} SlidingWindow5;


PIDcontroller pid;
SlidingWindow5 filteredSpeedData; // filtered speed data struct instance


Task_Struct motorTaskStruct;
Char motorTaskStack[TASKSTACKSIZE];


// clocks
Clock_Params clockParams;
Clock_Handle speedCalc_ClockHandler;
Clock_Handle speedFilter_ClockHandler;
Clock_Handle speedControl_ClockHandler;
Clock_Handle accelLimit_ClockHandler;


// error block
Error_Block eb;


// event handle - testing purposes
Event_Handle motor_evtHandle;


//UART HWI handle - for testing
Hwi_Handle UartHwiHandle_p;


extern void hallSensorFxn();

extern void speedCalc_ClockHandlerFxn();

extern void speedFilter_ClockHandlerFxn();

extern void PIDControllerInit();

extern void speedControl_ClockHandlerFxn();

extern void accelLimit_ClockHandlerFxn();

extern void toggleMotorClocks();

extern void startMotorRoutine();

extern void stopMotorRoutine(bool estop);

extern void motorTestFxn(); // testing

extern void motorOperation();

extern void initMotorTasks();

extern void initMotorDriver();

extern void UARTFxn(); // testing

extern void initUART(); // testing


#endif










