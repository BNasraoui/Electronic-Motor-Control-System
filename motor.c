#include "motor.h"

// delete this once finalised
double PID_KP =             1.3;
double PID_KI =             0.0001;
double PID_KD  =            0.45; // Kp = 1.35, Ki = 0.05, Kd = 0.45


// motor conditions
bool motorRunning           = true; // current motor status
bool estopGUI               = false; // E-Stop condition flag for GUI


// motor params
int pwmPeriod               = 24; // recommended period
int dutyCycle               = 0;


// speed sensing variables
uint16_t interruptCounter   = 0;
double desiredSpeed         = 2000; // value between 0 and 5820 RPM (0 - 100% duty cycle)
double intermediateSpeed    = 0; // value determined by the PID controller
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
    pid.T = PID_PERIOD;

    // derivative filter time constant
    pid.tau = PID_TAU; // as the number approaches 0, the lesser the filtering

    // set integrator limits - calculated in the loop
//    pid.limMinInt = PID_LIM_MIN_INT;
//    pid.limMaxInt = PID_LIM_MAX_INT;

    // set speed limits
    pid.maxSpeed = PID_MAX_SPEED;
    pid.minSpeed = PID_MIN_SPEED;

    // clear all controller variables
    pid.integrator = 0;
    pid.previousError = 0;
    pid.differentiator = 0;
    pid.previousSpeed = 0;
    pid.outputSpeed = 0;
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

    rawSpeed = ((interruptCounter * 10) / 24) * 60; // raw speed - eqn verified through physical measurement
    interruptCounter = 0; // reset counter
}

// Clock callback function to filter the motor speed data every 10ms (100Hz)
void speedFilter_ClockHandlerFxn()
{
    filteredSpeedData.sum = filteredSpeedData.sum - filteredSpeedData.data[filteredSpeedData.index];
    filteredSpeedData.data[filteredSpeedData.index] = rawSpeed;
    filteredSpeedData.sum = filteredSpeedData.sum + filteredSpeedData.data[filteredSpeedData.index];
    filteredSpeedData.index = (filteredSpeedData.index + 1) % WINDOW_SIZE;
    filteredSpeedData.avg = filteredSpeedData.sum / WINDOW_SIZE;    // filtered speed
}

double proportional;

// Clock callback function to control the motor speed via a PID controller (50Hz)
void speedControl_ClockHandlerFxn()
{
    // value between 0 and 5820 RPM (0 - 100% duty cycle = 0 - 24)
    // thus, % duty cycle = (RPM / 58.20) * 0.24 = (RPM / 242.5)

    // error signal
    double error = intermediateSpeed - filteredSpeedData.avg;

    // proportional
    proportional = pid.Kp * error;

    // Integral
    pid.integrator = (pid.integrator + (pid.Ki *error)); // * pid.T)); //    pid.integrator = pid.integrator + 0.5 * pid.Ki * pid.T * (error + pid.previousError);

//    if (pid.maxSpeed > proportional)
//    {
//        pid.limMaxInt = pid.maxSpeed - proportional;
//    }
//    else
//    {
//        pid.limMaxInt = 0;
//    }
//
//    if (pid.minSpeed < proportional)
//    {
//        pid.limMinInt = pid.minSpeed - proportional;
//    }
//    else
//    {
//        pid.limMinInt = 0;
//    }
//
//    // Anti-wind-up (clamp the integrator)
//    if(pid.integrator > pid.limMaxInt)
//    {
//        pid.integrator = pid.limMaxInt;
//    }
//    else if (pid.integrator < pid.limMinInt)
//    {
//        pid.integrator = pid.limMinInt;
//    }

//    // Derivative (band-limited differentiator)
//    pid.differentiator = -(2.0 * pid.Kd * (filteredSpeedData.avg - pid.previousSpeed)
//                       + (2.0 * pid.tau - pid.T) * pid.differentiator)
//                       / (2.0 * pid.tau + pid.T);

    // compute output and apply limits

    pid.outputSpeed = proportional + pid.integrator;  //+ pid.differentiator;

    if (pid.outputSpeed > pid.maxSpeed)
    {
        pid.outputSpeed = pid.maxSpeed;
    }
    else if ( pid.outputSpeed < pid.minSpeed)
    {
        pid.outputSpeed = pid.minSpeed;
    }

    // set duty cycle
    setDuty(pid.outputSpeed / 242.5); //desiredSpeed / 242.5); //

    // store error and measurement for next itteration
    pid.previousError = error;
    pid.previousSpeed = filteredSpeedData.avg;


}

// set the acceleration limits
void accelLimit_ClockHandlerFxn() // call at < 500Hz // call 1000Hz if not estop then + 0.5 if estop then + 1 etc // startup issue may because of filteredData.avg = 0 on start
{
    // use intermediate value between desired speed and controlled speed
    // increment or decrement the value at a certain rate
    // this value will be the desired speed we input into the PID controller

    // if not estop then increment/decrement the speed at no greater than 500/s
    // if estop then decrement at 1000/s.


    // i measured this and its only 250 RPM/s when we call the function 500 times a second


    if (intermediateSpeed < desiredSpeed)
    {
        intermediateSpeed = intermediateSpeed + 1;
    }
    else if (intermediateSpeed > desiredSpeed)
    {
        intermediateSpeed = intermediateSpeed - 1;
    }
    else
    {
         // do nothing
    }

}


// toggle clocks based on the motor operation
void toggleMotorClocks()
{
    if (motorRunning)
    {
        Clock_start(speedCalc_ClockHandler);
        Clock_start(speedFilter_ClockHandler);
        Clock_start(accelLimit_ClockHandler);
        Clock_start(speedControl_ClockHandler);

    }
    else
    {
        Clock_stop(speedCalc_ClockHandler);
        Clock_stop(speedFilter_ClockHandler);
        Clock_stop(accelLimit_ClockHandler);
        Clock_stop(speedControl_ClockHandler);
    }
}

// FOR TESTING
void motorTestFxn()
{
    if (motorRunning)
    {
        Event_post(motor_evtHandle, STOP_MOTOR);
        System_printf("[INFO] stopping motor\n\n");
        System_flush();
    }
    else
    {
        Event_post(motor_evtHandle, START_MOTOR);
        System_printf("[INFO] starting motor\n\n");
        System_flush();
    }
}

void startMotorRoutine()
{

    motorRunning = true;

    setDuty((desiredSpeed / 242.5)); // set initial duty cycle to start the motor

    // read the hall sensors
    Hall_a = GPIO_read(Board_Hall_A);
    Hall_b = GPIO_read(Board_Hall_B);
    Hall_c = GPIO_read(Board_Hall_C);

    // force one step of the motor using the update function
    updateMotor(Hall_a, Hall_b, Hall_c);

    //toggleMotorClocks(); // start all clocks
}

void stopMotorRoutine(bool estop)
{
    motorRunning = false;
    //toggleMotorClocks(); // stop all clocks
    stopMotor(false); // stop the motor using motor lib

    if (estop)
    {
        estopGUI = true; // set flag for GUI
    }

}

// intialise the task and clocks used by the motor driver
void initMotorTasks()
{
    Task_Params taskParams;

    // motor operation Task thread
    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &motorTaskStack;
    Task_construct(&motorTaskStruct, (Task_FuncPtr)motorOperation, &taskParams, NULL);

    // speed calculation loop at 10Hz
    Clock_Params_init(&clockParams);
    clockParams.period = CLOCK_PERIOD_10Hz;
    clockParams.startFlag = FALSE; //start and stop when were running the motor
    speedCalc_ClockHandler = Clock_create((Clock_FuncPtr)speedCalc_ClockHandlerFxn, CLOCK_TIMEOUT_MS, &clockParams, &eb);
    if (speedCalc_ClockHandler == NULL) {
     System_abort("speed calculation clock handle create failed");
    }

    // speed filtering loop at 100Hz - as per system requirements
    Clock_Params_init(&clockParams);
    clockParams.period = CLOCK_PERIOD_100Hz;
    clockParams.startFlag = FALSE; // start and stop when were running the motor
    speedFilter_ClockHandler = Clock_create((Clock_FuncPtr)speedFilter_ClockHandlerFxn, CLOCK_TIMEOUT_MS, &clockParams, &eb);
    if (speedFilter_ClockHandler == NULL) {
     System_abort("speed calculation clock handle create failed");
    }

    // speed control loop at 50Hz
    Clock_Params_init(&clockParams);
    clockParams.period = CLOCK_PERIOD_50Hz;
    clockParams.startFlag = FALSE; // start and stop when were running the motor
    speedControl_ClockHandler = Clock_create((Clock_FuncPtr)speedControl_ClockHandlerFxn, CLOCK_TIMEOUT_MS, &clockParams, &eb);
    if (speedControl_ClockHandler == NULL) {
     System_abort("speed control clock handle create failed");
    }

    // acceleration limit loop at 500Hz
    Clock_Params_init(&clockParams);
    clockParams.period = CLOCK_PERIOD_500Hz;
    clockParams.startFlag = FALSE; // start and stop when were running the motor
    accelLimit_ClockHandler = Clock_create((Clock_FuncPtr)accelLimit_ClockHandlerFxn, CLOCK_TIMEOUT_MS, &clockParams, &eb);
    if (speedControl_ClockHandler == NULL) {
     System_abort("acceleration limit clock handle create failed");
    }

}

// initialise the motor driver
void initMotorDriver()
{
    // board init functions - will need to remove for integration
    Board_initGeneral();
    Board_initGPIO();
    Board_initUART();
    initUART(); // testing

    // init the error block - will need to remove for integration
    Error_init(&eb);

    initMotorTasks(); // setup tasks

    PIDControllerInit();// setup the pid controller

    // Create an Event object - testing purposes, remove of integration?
    motor_evtHandle = Event_create(NULL, &eb);
    if (motor_evtHandle == NULL)
    {
        System_abort("Event create failed");
    }

    // init the motor library and check for errors
    bool success = initMotorLib(pwmPeriod, &eb);

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

    // functionality testing - to be removed
    GPIO_setCallback(Board_BUTTON0, (GPIO_CallbackFxn)motorTestFxn);
    GPIO_enableInt(Board_BUTTON0);

    // start all the clocks
    Clock_start(speedCalc_ClockHandler);
    Clock_start(speedFilter_ClockHandler);
    Clock_start(accelLimit_ClockHandler);
    Clock_start(speedControl_ClockHandler);

    System_printf("[INFO] Motor subsystem initalisation complete\n\n");
    System_flush();

}


// FOR TESTING
void UARTFxn()
{

    // Get and clear interrupt status.
    uint32_t intStatus;
    intStatus = UARTIntStatus(0x4000C000);
    UARTIntClear(0x4000C000, intStatus);

    size_t i = 0;

    char c = (char)UARTCharGetNonBlocking(0x4000C000);

    if (c == 'a')
    {
        PID_KP = PID_KP + 0.01;
        System_printf("[INFO] Kp = %f\n\n", PID_KP);
        System_flush();
    }
    if (c == 's')
    {
        PID_KP = PID_KP - 0.01;
        System_printf("[INFO] Kp = %f\n\n", PID_KP);
        System_flush();
    }
    if (c == 'd')
    {
        PID_KI = PID_KI + 0.0001;
        System_printf("[INFO] Ki = %f\n\n", PID_KI);
        System_flush();
    }
    if (c == 'f')
    {
        PID_KI = PID_KI - 0.0001;
        System_printf("[INFO] Ki = %f\n\n", PID_KI);
        System_flush();
    }
    if (c == 'g')
    {
        PID_KD = PID_KD + 0.01;
        System_printf("[INFO] Kd = %f\n\n", PID_KD);
        System_flush();
    }
    if (c == 'h')
    {
        PID_KD = PID_KD - 0.01;
        System_printf("[INFO] Kd = %f\n\n", PID_KD);
        System_flush();
    }

}


// FOR TESTING
void initUART()
{

    //Create Hwi for UART interrupt
    Hwi_Params hwiParams;
    Hwi_Params_init(&hwiParams);

    //need to find the interrupt number for the uart and replace at XX --> 39 (Timer 2A)
    UartHwiHandle_p = Hwi_create(21, (Hwi_FuncPtr)UARTFxn , &hwiParams, NULL);

    if (UartHwiHandle_p == NULL) {
        System_abort("Hwi create failed");
    }

    Types_FreqHz cpuFreq;
    BIOS_getCpuFreq(&cpuFreq);

    //(0x4000C000) base addr of the UART port used UART0_BASE
    uint32_t baud_rate = 115200; // baus rate of the UART port

    UARTConfigSetExpClk(0x4000C000, (uint32_t)cpuFreq.lo, baud_rate, 0x00000060 | 0x00000000 | 0x00000000);

    //Need to turn on the uart,
    UARTEnable(0x4000C000);

    //This enables the interrupt for uart in the vector table
    IntEnable(INT_UART0);

    //configure the what type of interrupt you want to cause an interrupt on the uart
    //Need to choose the right config values for the interrupt behaviours you want
    //Have a look in driverlib/uart.h for some predefines for these settings
    UARTIntEnable(0x4000C000, 0x040 | 0x020);
}

