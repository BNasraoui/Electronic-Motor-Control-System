#ifndef __SENSORS_H__
#define __SENSORS_H__

/* XDCtools Header files */
#include <xdc/std.h>
#include <file.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <xdc/runtime/Types.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Timestamp.h>
#include <xdc/runtime/Error.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/gates/GateHwi.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Swi.h>
#include <ti/sysbios/hal/Timer.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Event.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/Watchdog.h>

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/adc.h"
#include "driverlib/timer.h"

/* Tiva C series macros header files */
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"

/* Board Header file */
#include "Board.h"
#include "general.h"

#define GPIO_PORTP_BASE                 0x40065000  // GPIO Port P
#define GPIO_PIN_2                      0x00000004  // GPIO pin 2
#define P2_VECTOR_NUM                   94

#define ADC0_SEQ1_VEC_NUM               31
#define ADC1_SEQ1_VEC_NUM               63
#define ADC_SEQ                         1
#define ADC_STEP                        0
#define WINDOW_SIZE                     5
#define ADC_RESOLUTION                  0.0008
#define SHUNT_R_VALUE                   0.007

#define CLOCK_PERIOD_150HZ              6    //6ms = ~150Hz
#define CLOCK_PERIOD_2HZ                500  //500ms = 2Hz
#define CLOCK_PERIOD_1HZ                1000
#define CLOCK_PERIOD_10HZ               100
#define CLOCK_TIMEOUT_MS                   10  //ms

#define LOW_HIGH_LIGHT_EVENT            Event_Id_00
#define NEW_OPT3001_DATA                Event_Id_01
#define NEW_ACCEL_DATA                  Event_Id_02
#define NEW_ADC0_DATA                   Event_Id_03
#define NEW_ADC1_DATA                   Event_Id_04

#define TURN_HEADLIGHTS_ON              1
#define TURN_HEADLIGHTS_OFF             0
#define NIGHTTIME_LUX_VAL               5
#define ON                              1
#define OFF                             0

typedef struct Sliding_Window32 {
    uint8_t index;
    bool startFilter;
    uint32_t sum;
    float avg;
    float voltage;
    float current;
    float power;
    uint32_t data[WINDOW_SIZE];
} SlidingWindow_32;

typedef struct Sliding_Window_16 {
    uint8_t index;
    bool startFilter;
    int32_t sum;
    float avg;
    float G;
    int16_t data[WINDOW_SIZE];
} SlidingWindow_16;

typedef struct Sliding_Window_u16 {
    uint8_t index;
    bool startFilter;
    uint32_t sum;
    float avg;
    uint16_t data[WINDOW_SIZE];
} SlidingWindow_u16;

float absoluteAccel;

Hwi_Handle hwi_ADC0;
Hwi_Handle hwi_ADC1;
Hwi_Params hwiParams;
GateHwi_Handle gateHwi;
GateHwi_Params gHwiprms;

Swi_Params swiParams;
Swi_Struct swi0Struct, swi1Struct, swi2Struct, swi3Struct;
Swi_Handle swiHandle_ADC0DataProc, swiHandle_ADC1DataProc, swiHandle_accelDataProc, swiHandle_LuxDataProc;

Error_Block eb;

I2C_Handle i2cHandle;
I2C_Params i2cParams;
I2C_Transaction i2cTransactionCallback;
uint8_t rxBuffer_BMI[6];

Clock_Params clockParams;
Clock_Handle adc_ClockHandler;
Clock_Handle opt3001_ClockHandler;
Clock_Handle watchDog_ClockHandler;
Clock_Handle widgetQueue_ClockHandler;

Watchdog_Handle watchDogHandle;

SlidingWindow_32 ADC0Window;
SlidingWindow_32 ADC1Window;
SlidingWindow_16 accelXFilt;
SlidingWindow_16 accelYFilt;
SlidingWindow_16 accelZFilt;

bool headLightState;

extern void WatchDogBite();

extern void ADC_ClockHandlerFxn();

extern void UpdateWidgetQueue();

extern void TaskStatusCheck();

extern void InitSensorDriver();

extern void InitInterrupts();

extern void InitADC1_CurrentSense();

extern void ProcessSensorEvents();

extern void ReadSensorsFxn();

extern void ADC0_FilterFxn();

extern void ADC1_FilterFxn();

extern void BMI160Fxn();

extern void OPT3001Fxn();

extern void ProcessAccelDataFxn();

extern void ADC0_Read();

extern void ADC1_Read();

extern float GetLightLevel();

//extern void GetLightLevel();

extern void GetAccelData();

extern void BufferReadI2C_OPT3001(uint8_t slaveAddress, uint8_t ui8Reg);

extern void BufferReadI2C_BMI160(uint8_t slaveAddress, uint8_t ui8Reg);

extern void BufferReadResultI2C_OPT3001(uint8_t slaveAddress, uint8_t ui8Reg);

extern void I2C_Callback(I2C_Handle handle, I2C_Transaction *i2cTransaction, bool result);

extern bool ReadHalfWordI2C(I2C_Handle i2cHandle,uint8_t slaveAddress, uint8_t ui8Reg, uint8_t *data);

extern bool ReadByteI2C(I2C_Handle i2cHandle,uint8_t slaveAddress, uint8_t ui8Reg, uint8_t *data);

extern bool WriteHalfwordI2C(I2C_Handle i2cHandle, uint8_t slaveAddress, uint8_t ui8Reg, uint8_t *data);

extern bool WriteByteI2C(I2C_Handle i2cHandle, uint8_t slaveAddress, uint8_t ui8Reg, uint8_t data);

#endif // __SENSORS_H__
