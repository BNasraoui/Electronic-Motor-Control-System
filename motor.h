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

#define TASKSTACKSIZE       1024

#define CLOCK_PERIOD_200Hz  5 // 5ms = 200Hz -> Speed Calculation
#define CLOCK_PERIOD_1000Hz 1  // 1ms = 1000Hz -> PID loop
#define CLOCK_PERIOD_500Hz  2   // 2ms = 500Hz -> acceleration limit
#define CLOCK_TIMEOUT_MS    10  // 10ms

#define WINDOW_SIZE         20   // filter window size

#define PID_PERIOD          0.001 // 1ms or 1000Hz

#define PID_MIN_SPEED       0
#define PID_MAX_SPEED       5820 // no load speed at max duty cycle (measured)

#define PID_LIM_MIN_INT     -5.0
#define PID_LIM_MAX_INT     5.0


typedef struct {

    // controller gains
    double Kp;
    double Ki;
    double Kd;

    // output limits
    double maxSpeed;
    double minSpeed;

    // Integrator limits
    float limMinInt;
    float limMaxInt;

    // sample time in secs
    double dT;

    // controller memory
    double integrator;
    double prevError;

    // controller output
    double outputSpeed;

} PIDcontroller;


typedef struct {
    uint8_t index;
    double sum;
    double avg;
    double data[WINDOW_SIZE];

} SlidingWindow20;


PIDcontroller pid; // PID controller struct instance
SlidingWindow20 filteredSpeedData; // filter struct instance


Task_Struct motorTaskStruct;
Char motorTaskStack[TASKSTACKSIZE];


// clocks
Clock_Params clockParams;
Clock_Handle speedCalc_ClockHandler;
Clock_Handle accelLimit_ClockHandler;
Clock_Handle speedControl_ClockHandler;
Clock_Handle hasMotorStopped_ClockHandler;


// error block
Error_Block eb;


// event handle - testing purposes
Event_Handle motor_evtHandle;


//UART HWI handle - for testing
Hwi_Handle UartHwiHandle_p;

// project globals
extern double desiredSpeed; // value between 0 and 5820 RPM (0 - 100% duty cycle)
extern bool estopFlag;


extern void hallSensorFxn();

extern void PIDControllerInit();

extern void speedCalc_ClockHandlerFxn();

extern void speedFilter_ClockHandlerFxn();

extern void speedControl_ClockHandlerFxn();

extern void accelLimit_ClockHandlerFxn();

extern void hasMotorStopped_ClockHandlerFxn();

extern void toggleMotorClocks();

extern void startMotorRoutine();

extern void changeSpeedRoutine(double newSpeed, bool estop);

extern void motorTestFxn(); // testing
extern void motorTestFxnOne(); // etsting

extern void motorOperation();

extern void initMotorTasks();

extern void initMotorDriver();


#endif










