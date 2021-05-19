#ifndef __SENSORS_H__
#define __SENSORS_H__

/* XDCtools Header files */
#include <xdc/std.h>
#include <file.h>
#include <stdio.h>
#include <string.h>

#include <xdc/runtime/Types.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/gates/GateHwi.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Swi.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/I2C.h>

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

//OPT3001 slave address
#define OPT3001_SLAVE_ADDRESS           0x47
#define BMI160_SLAVE_ADDRESS            0x69
#define GPIO_PORTP_BASE                 0x40065000  // GPIO Port P
#define GPIO_PIN_2                      0x00000004  // GPIO pin 2

/* OP3001 Register addresses */
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
#define TASKSTACKSIZE                   512

#define P2_VECTOR_NUM              94
#define ADC0_SEQ1_VEC_NUM          31
#define ADC1_SEQ1_VEC_NUM          63
#define ADC_SEQ                     1
#define ADC_STEP                    0
#define WINDOW_SIZE             5

typedef struct Sliding_Window32{
    uint8_t index;
    uint32_t sum;
    uint32_t avg;
    uint32_t data[WINDOW_SIZE];
} SlidingWindow32;

typedef struct Sliding_Window16{
    uint8_t index;
    uint32_t sum;
    uint32_t avg;
    uint16_t data[WINDOW_SIZE];
} SlidingWindow16;

//Moving average filtering with buffer
SlidingWindow32 ADC0Window;
SlidingWindow32 ADC1Window;
SlidingWindow16 accelXFilt;
SlidingWindow16 accelYFilt;
SlidingWindow16 accelZFilt;

Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];

Hwi_Handle hwi_OPT3001;
Hwi_Handle hwi_ADC0;
Hwi_Handle hwi_ADC1;
Hwi_Params hwiParams;
GateHwi_Handle gateHwi;
GateHwi_Params gHwiprms;

Swi_Params swiParams;
Swi_Struct swi0Struct, swi1Struct, swi2Struct;
Swi_Handle swi0Handle, swi1Handle, swi2Handle;
Semaphore_Struct sem0Struct;
Semaphore_Handle sem0Handle;

I2C_Handle i2cHandle;
I2C_Params i2cParams;
I2C_Handle bmi160Handle;
bool clearingIntBit;

I2C_Transaction i2cTransaction_BMI;
uint8_t rxBuffer_BMI[6];
uint8_t txBuffer_BMI[1];

I2C_Transaction i2cTransaction_OPT;
uint8_t rxBuffer_OPT[2];
uint8_t txBuffer_OPT[1];

bool lightLimitReached;

typedef struct Sensors_Config      *Sensors_Handle;

typedef struct SensorsTiva_Object {

}SensorsTiva_Object;

/*!
 *  @brief  SENSORS Parameters
 *
 *  Params used to initialise the sensors driver
 *
 */
typedef struct Sensors_Params_Params {
   // Sensors_Params_TransferMode    transferMode; /*!< Blocking or Callback mode */
   // Sensors_Params_CallbackFxn     transferCallbackFxn; /*!< Callback function pointer */
   // Sensors_Params_BitRate         bitRate; /*!< I2C bus bit rate */
  //  uintptr_t           custom;  /*!< Custom argument used by driver implementation */
} Sensors_Params;

/*!
 *
 */
typedef struct SENSORS_FxnTable {
    /*! Function to close the specified peripheral */
   // SENSORS_CloseFxn        closeFxn;

    /*! Function to implementation specific control function */
   // SENSORS_ControlFxn      controlFxn;

    /*! Function to initialize the given data object */
   // SENSORS_InitFxn         initFxn;

    /*! Function to open the specified peripheral */
    //SENSORS_OpenFxn         openFxn;

    /*! Function to initiate a I2C data transfer */
    //SENSORS_TransferFxn     transferFxn;
} SENSORS_FxnTable;

/*!
 *
 *
 */
typedef struct Sensors_Config {
    /*! Pointer to a table of driver-specific implementations of I2C APIs */
    I2C_FxnTable const *fxnTablePtr;

    /*! Pointer to a driver specific data object */
    void               *object;

    /*! Pointer to a driver specific hardware attributes structure */
    void         const *hwAttrs;
} Sensors_Config;

extern void InitSensorDriver();

extern void ReadSensorsFxn();

extern void InitI2C_opt3001();

extern void InitI2C_BMI160();

extern void InitADC0_CurrentSense();

extern void InitADC1_CurrentSense();

extern void ADC0_Read();

extern void ADC1_Read();

extern void ADC0_FilterFxn();

extern void ADC1_FilterFxn();

extern bool SensorOpt3001Read(I2C_Handle opt3001, uint16_t *rawData);

extern bool SensorBMI160_GetAccelData(uint16_t *accelX, uint16_t *accelY, uint16_t *accelZ);

extern void ProcessAccelDataFxn();

extern void I2C_Callback(I2C_Handle handle, I2C_Transaction *i2cTransaction, bool result);

extern void SensorOpt3001Convert(uint16_t rawData, float *convertedLux);

extern void SetLowLimit_OPT3001(float val);

extern void SetHighLimit_OPT3001(float val);

extern uint16_t CalculateLimitReg(float luxValue);

extern bool BufferReadI2C_OPT3001(I2C_Handle handle, uint8_t slaveAddress, uint8_t ui8Reg, int numBytes);

extern bool BufferReadI2C_BMI160(I2C_Handle handle, uint8_t slaveAddress, uint8_t ui8Reg, int numBytes);

extern bool ReadI2C(I2C_Handle i2cHandle,uint8_t slaveAddress, uint8_t ui8Reg, uint8_t *data);

extern bool ReadByteI2C(I2C_Handle i2cHandle,uint8_t slaveAddress, uint8_t ui8Reg, uint8_t *data);

extern bool WriteHalfwordI2C(I2C_Handle i2cHandle, uint8_t slaveAddress, uint8_t ui8Reg, uint8_t *data);

extern bool WriteByteI2C(I2C_Handle i2cHandle, uint8_t slaveAddress, uint8_t ui8Reg, uint8_t data);

extern void OPT3001Fxn();

extern void BMI160Fxn();

#endif // __TOUCH_H__

typedef struct SensorsTiva_HWAttrs {
    /*! I2C Peripheral's base address */
    unsigned int baseAddr;
    /*! I2C Peripheral's interrupt vector */
    unsigned int intNum;
    /*! I2C Peripheral's interrupt priority */
    unsigned int intPriority;
} SensorsTiva_HWAttrs;
