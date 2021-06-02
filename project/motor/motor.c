#include "motor/motor.h"
#include "general.h"

// motor params
int pwmPeriod               = 24;    // recommended period
int dutyCycle               = 0;


// speed sensing variables
uint16_t interruptCounter   = 0;
double intermediateSpeed    = 0;     // value determined by the PID controller
double rawSpeed             = 0;

// hall effect sensor states
bool Hall_a                 = false;
bool Hall_b                 = false;
bool Hall_c                 = false;


// intialise the PID controller
void PIDControllerInit()
{
    // set controller gains
    pid.Kp = PID_KP;
    pid.Ki = PID_KI;
    pid.Kd = PID_KD;

    // sample period 20ms or 0.02s or 50Hz
    pid.dT = PID_PERIOD;

    // set speed limits
    pid.maxSpeed = PID_MAX_SPEED;
    pid.minSpeed = PID_MIN_SPEED;

    // clear all controller variables
    pid.integrator = 0; // test at 3k RPM
    pid.prevError = 0;
    pid.outputSpeed = 0;
}

void filterInit()
{
    filteredSpeedData.startFilter = false;
    filteredSpeedData.sum = 0;
    filteredSpeedData.avg = 0;
}

// HWI callback function for hall sensor hall sensor edge change
void hallSensorFxn()
{
    // handle motor operation only if the motor running flag is set
    if (motorRunning)
    {
        // read the state of the GPIO lines
        Hall_a = GPIO_read(Board_Hall_A);
        Hall_b = GPIO_read(Board_Hall_B);
        Hall_c = GPIO_read(Board_Hall_C);

        interruptCounter++; // increment speed calc counter

        updateMotor(Hall_a, Hall_b, Hall_c); // update motor phases
    }
}

// Clock callback function to calculate speed of the motor every 100ms
void speedCalc_ClockHandlerFxn()
{
    // interrupts / 100ms --> extrapolate to interrupts/second by multiplying by 10
    // for each hall sensor value change we have 60 degrees of electrical rotation
    // 60 degree of electrical rotation corresponds to 15 degrees mechanical for an 8 pole motor
    // thus, to get rev/s divide by 360/15 = 24
    // multiply by 60 to get RPM

    rawSpeed = ((interruptCounter * 200) / 24) * 60; // raw speed - eqn verified through physical measurement
    interruptCounter = 0; // reset counter

    // filter the speed
    filteredSpeedData.sum = filteredSpeedData.sum - filteredSpeedData.data[filteredSpeedData.index];
    filteredSpeedData.data[filteredSpeedData.index] = rawSpeed;
    filteredSpeedData.sum = filteredSpeedData.sum + filteredSpeedData.data[filteredSpeedData.index];
    filteredSpeedData.index = (filteredSpeedData.index + 1) % WINDOW_SIZE_SPEED;

    // if the buffer is full then allow access to the filtered data
    if (filteredSpeedData.startFilter)
    {
        filteredSpeedData.avg = filteredSpeedData.sum / WINDOW_SIZE_SPEED;    // filtered speed
    }
    else
    {
        filteredSpeedData.avg = rawSpeed;
    }

    if ((filteredSpeedData.index + 1 == WINDOW_SIZE_SPEED) && (filteredSpeedData.startFilter == false))
    {
        filteredSpeedData.startFilter = true;
    }

    Event_post(GU_eventHandle, EVENT_GRAPH_SPEED);

}


// Clock callback function to control the motor speed via a PID controller (1000Hz)
void speedControl_ClockHandlerFxn()
{
    // value between 0 and 5820 RPM (0 - 100% duty cycle = 0 - 24)
    // thus, % duty cycle = (RPM / 58.20) * 0.24 = (RPM / 242.5)

    // error signal
    double error = intermediateSpeed - filteredSpeedData.avg;

    double proportional = pid.Kp * error;

    // Integral
    pid.integrator = pid.integrator + (error * pid.dT);

    // differential
    double differentiator =  (error - pid.prevError ) / pid.dT;

    // compute output
    pid.outputSpeed = proportional + pid.Ki * pid.integrator - pid.Kd * differentiator;

    // apply limits
    if (pid.outputSpeed > pid.maxSpeed)
    {
        pid.outputSpeed = pid.maxSpeed;
    }
    else if (pid.outputSpeed < pid.minSpeed)
    {
        pid.outputSpeed = pid.minSpeed;
    }

    // set duty cycle
    setDuty(pid.outputSpeed / 242.5);

    // store the error
    pid.prevError = error;
}

// set the acceleration limits
void accelLimit_ClockHandlerFxn()
{
    // I measured this and its only 250 RPM/s when we call the function 500 times a second??
    // however, does produce accurate results - 250 RPM/s and 1000 RPM/s for estop

    if (intermediateSpeed < desiredSpeed)
    {
        intermediateSpeed = intermediateSpeed + 1;
    }
    else if (intermediateSpeed > desiredSpeed)
    {
        if (estopFlag)
        {
            intermediateSpeed = intermediateSpeed - 4; // 250 RPM/s * 4 = 1000 RPM/s
        }
        else
        {
            intermediateSpeed = intermediateSpeed - 1;
        }
    }
    else
    {
        // intermediate = desired (do nothing)
    }

}


// toggle clocks based on the motor operation
void toggleMotorClocks()
{
    if (motorRunning)
    {
        Clock_start(accelLimit_ClockHandler);
        Clock_start(speedControl_ClockHandler);
    }
    else
    {
        intermediateSpeed = 0; // reset the intermediate speed variable each time the motor starts
        Clock_stop(accelLimit_ClockHandler);
        Clock_stop(speedControl_ClockHandler);
    }
}


void startMotorRoutine()
{
    desiredSpeed = userSpeed;
    intermediateSpeed = 700;// need this or it doesnt start

    estopFlag = false;
    motorRunning = true;

    toggleMotorClocks(); // start all clocks

    setDuty(10); // set initial duty cycle to start the motor
    // need the PID to dictate the duty cycle
    // read the hall sensors
    Hall_a = GPIO_read(Board_Hall_A);
    Hall_b = GPIO_read(Board_Hall_B);
    Hall_c = GPIO_read(Board_Hall_C);

    // force one step of the motor using the update function
    updateMotor(Hall_a, Hall_b, Hall_c);

    Task_sleep(1000); // delay to let the motor start before we start the has motor stopped SWI

    Clock_start(hasMotorStopped_ClockHandler); // start the check motor status loop
}


// check if the motor has stopped then complete the stopped motor routine (1Hz)
void hasMotorStopped_ClockHandlerFxn()
{
    if (filteredSpeedData.avg <= 0) // not letting the motor start
    {
        motorRunning = false; // set flag

        toggleMotorClocks(); // stop all clocks

        stopMotor(false); // ensure motor has stopped using the motorlib

        Clock_stop(hasMotorStopped_ClockHandler); // once the motor has stopped then dont check if it has stopped
    }
}


// intialise the task and clocks used by the motor driver
void initMotorTasks()
{

    // speed calculation loop at 200Hz
    Clock_Params_init(&clockParams);
    clockParams.period = CLOCK_PERIOD_200Hz;
    clockParams.startFlag = FALSE; //start and stop when were running the motor
    speedCalc_ClockHandler = Clock_create((Clock_FuncPtr)speedCalc_ClockHandlerFxn, CLOCK_TIMEOUT_MS, &clockParams, NULL);
    if (speedCalc_ClockHandler == NULL)
    {
        System_abort("speed calculation clock handle create failed");
    }

    // speed control loop at 1000Hz
    Clock_Params_init(&clockParams);
    clockParams.period = CLOCK_PERIOD_1000Hz;
    clockParams.startFlag = FALSE; // start and stop when were running the motor
    speedControl_ClockHandler = Clock_create((Clock_FuncPtr)speedControl_ClockHandlerFxn, CLOCK_TIMEOUT_MS, &clockParams, NULL);
    if (speedControl_ClockHandler == NULL)
    {
        System_abort("speed control clock handle create failed");
    }

    // acceleration limit loop at 500Hz
    Clock_Params_init(&clockParams);
    clockParams.period = CLOCK_PERIOD_500Hz;
    clockParams.startFlag = FALSE; // start and stop when were running the motor
    accelLimit_ClockHandler = Clock_create((Clock_FuncPtr)accelLimit_ClockHandlerFxn, CLOCK_TIMEOUT_MS, &clockParams, NULL);
    if (accelLimit_ClockHandler == NULL)
    {
        System_abort("acceleration limit clock handle create failed");
    }

    // has motor stopped?
    Clock_Params_init(&clockParams);
    clockParams.period = 500; // twice a second
    clockParams.startFlag = FALSE; // always have running
    hasMotorStopped_ClockHandler = Clock_create((Clock_FuncPtr)hasMotorStopped_ClockHandlerFxn, CLOCK_TIMEOUT_MS, &clockParams, NULL);
    if (hasMotorStopped_ClockHandler == NULL)
    {
        System_abort("has motor stopped clock handle create failed");
    }

}

// initialise the motor driver
void initMotorDriver()
{
    initMotorTasks(); // setup tasks

    filterInit(); // setup the filter

    PIDControllerInit();// setup the pid controller

    // motor conditions
    motorRunning = false; // current motor status
    estopFlag = false; // E-Stop condition flag for GUI and acceleration limits

    // init the motor library and check for errors
    bool success = initMotorLib(pwmPeriod, NULL);
    if (!success)
    {
        System_abort("Motor library init failed");
    }

    // init moving average filter
    filteredSpeedData.index = 0;
    filteredSpeedData.sum = 0;
    filteredSpeedData.avg = 0;

    // enable interrupts on the required ports - configured within the  EK_TM4C1294Xl.h and EK_TM4C1294Xl.c files
    GPIO_enableInt(Board_Hall_A); // Port M vector number = 88 - PM3
    GPIO_enableInt(Board_Hall_B); // Port H vector number = 48 - PH2
    GPIO_enableInt(Board_Hall_C); // Port N vector number = 89 - PN2

    // same callback fxn for each HWI
    GPIO_setCallback(Board_Hall_A, (GPIO_CallbackFxn)hallSensorFxn);
    GPIO_setCallback(Board_Hall_B, (GPIO_CallbackFxn)hallSensorFxn);
    GPIO_setCallback(Board_Hall_C, (GPIO_CallbackFxn)hallSensorFxn);

    Clock_start(speedCalc_ClockHandler); // start the speed calculation loop

    System_printf("[INFO] Motor subsystem initalisation complete\n\n");
    System_flush();

}



