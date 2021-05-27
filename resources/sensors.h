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

/* OP3001 Register addresses */
//OPT3001 slave address
#define OPT3001_SLAVE_ADDRESS           0x47
#define BMI160_SLAVE_ADDRESS            0x69

#define REG_RESULT                      0x00
#define REG_CONFIGURATION               0x01
#define REG_LOW_LIMIT                   0x02
#define REG_HIGH_LIMIT                  0x03

#define REG_MANUFACTURER_ID             0x7E
#define REG_DEVICE_ID                   0x7F

/* Register values */
#define MANUFACTURER_ID                 0x5449  // TI
#define DEVICE_ID                       0x3001  // Opt 3001
#define CONFIG_RESET                    0xC810
#define CONFIG_TEST                     0xCC10
#define CONFIG_ENABLE                   0x10C4 // 100 ms, continuous
#define CONFIG_DISABLE                  0x10C0 // 0xC010   - 100 ms, shutdown

#define CONFIG_VAL                      0x10C4 // latched window-style comparison and INT pin reports active low
#define LOW_LIMIT                       0xFF0F // Full-scale range (Lux) = 40.95
#define HIGH_LIMIT                      0xFF6F // Full scale range (Lux) = 2620.80

/* Bit values */
#define DATA_RDY_BIT                    0x0080  // Data ready
// #define TASKSTACKSIZE                   512

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
#define CLOCK_TIMEOUT_MS                   10  //ms

#define LOW_HIGH_LIGHT_EVENT            Event_Id_00
#define NEW_OPT3001_DATA                Event_Id_01
#define NEW_ACCEL_DATA                  Event_Id_02
#define NEW_ADC0_DATA                   Event_Id_03
#define NEW_ADC1_DATA                   Event_Id_04

typedef struct Sliding_Window32{
    uint8_t index;
    bool startFilter;
    uint32_t sum;
    float avg;
    float voltage;
    float current;
    float power;
    uint32_t data[WINDOW_SIZE];
} SlidingWindow_32;

typedef struct Sliding_Window_16{
    uint8_t index;
    bool startFilter;
    int32_t sum;
    float avg;
    float G;
    int16_t data[WINDOW_SIZE];
} SlidingWindow_16;

typedef struct Sliding_Window_u16{
    uint8_t index;
    bool startFilter;
    uint32_t sum;
    float avg;
    uint16_t data[WINDOW_SIZE];
} SlidingWindow_u16;

Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];

Hwi_Handle hwi_OPT3001;
Hwi_Handle hwi_ADC0;
Hwi_Handle hwi_ADC1;
Hwi_Params hwiParams;
GateHwi_Handle gateHwi;
GateHwi_Params gHwiprms;

Swi_Params swiParams;
Swi_Struct swi0Struct, swi1Struct, swi2Struct, swi3Struct;
Swi_Handle swiHandle_ADC0DataProc, swiHandle_ADC1DataProc, swiHandle_accelDataProc, swiHandle_LuxDataProc;

Event_Handle sensors_eventHandle;
Error_Block eb;

I2C_Handle i2cHandle;
I2C_Params i2cParams;
I2C_Transaction i2cTransactionCallback;
uint8_t rxBuffer_BMI[6];

Clock_Params clockParams;
Clock_Handle adc_ClockHandler;
Clock_Handle opt3001_ClockHandler;
Clock_Handle watchDog_ClockHandler;

Watchdog_Handle watchDogHandle;

SlidingWindow_32 ADC0Window;
SlidingWindow_32 ADC1Window;
SlidingWindow_u16 luxValueFilt;
SlidingWindow_16 accelXFilt;
SlidingWindow_16 accelYFilt;
SlidingWindow_16 accelZFilt;
int16_t accelX, accelY, accelZ;
uint16_t rawData;

extern void watchDogBite();

extern void taskStatusCheck();

extern void InitSensorDriver();

extern void InitInterrupts();

extern void ReadSensorsFxn();

extern void ProcessSensorEvents();

extern void InitI2C_OPT3001();

extern void InitI2C_BMI160();

extern void InitADC0_CurrentSense();

extern void InitADC1_CurrentSense();

extern void OPT3001_ClockHandlerFxn();

extern void ADC_ClockHandlerFxn();

extern void ADC0_FilterFxn();

extern void ADC1_FilterFxn();

extern void OPT3001Fxn();

extern void BMI160Fxn();

extern bool GetLuxValue_OPT3001(uint16_t *rawData);

extern bool GetAccelData_BMI160(int16_t *accelX, int16_t *accelY, int16_t *accelZ);

extern void ADC0_Read();

extern void ADC1_Read();

extern void ProcessAccelDataFxn();

extern void ConvertRawAccelToGs();

extern void ProcessLuxDataFxn();

extern void SensorOpt3001Convert(uint16_t rawData, float *convertedLux);

extern void SetLowLimit_OPT3001(float val);

extern void SetHighLimit_OPT3001(float val);

extern uint16_t CalculateLimitReg(float luxValue);

extern void BufferReadI2C_OPT3001(uint8_t slaveAddress, uint8_t ui8Reg);

extern void BufferReadI2C_BMI160(uint8_t slaveAddress, uint8_t ui8Reg);

extern void BufferReadResultI2C_OPT3001(uint8_t slaveAddress, uint8_t ui8Reg);

extern void I2C_Callback(I2C_Handle handle, I2C_Transaction *i2cTransaction, bool result);

extern bool ReadHalfWordI2C(I2C_Handle i2cHandle,uint8_t slaveAddress, uint8_t ui8Reg, uint8_t *data);

extern bool ReadByteI2C(I2C_Handle i2cHandle,uint8_t slaveAddress, uint8_t ui8Reg, uint8_t *data);

extern bool WriteHalfwordI2C(I2C_Handle i2cHandle, uint8_t slaveAddress, uint8_t ui8Reg, uint8_t *data);

extern bool WriteByteI2C(I2C_Handle i2cHandle, uint8_t slaveAddress, uint8_t ui8Reg, uint8_t data);

#endif // __SENSORS_H__
