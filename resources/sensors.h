#ifndef __SENSORS_H__
#define __SENSORS_H__

/* XDCtools Header files */
#include <xdc/std.h>
#include <file.h>
#include <stdio.h>
#include <string.h>


/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <xdc/runtime/Types.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/gates/GateHwi.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/UART.h>

/* Board Header file */
#include "Board.h"

//OPT3001 slave address
#define OPT3001_I2C_ADDRESS             0x47

/* Register addresses */
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

extern void InitI2C_opt3001(I2C_Handle* opt3001);

extern bool SensorOpt3001Read(I2C_Handle opt3001, uint16_t *rawData);

extern bool ReadI2C(I2C_Handle opt3001, uint8_t ui8Reg, uint16_t *data);

extern void SensorOpt3001Convert(uint16_t rawData, float *convertedLux);

#endif // __TOUCH_H__

typedef struct SensorsTiva_HWAttrs {
    /*! I2C Peripheral's base address */
    unsigned int baseAddr;
    /*! I2C Peripheral's interrupt vector */
    unsigned int intNum;
    /*! I2C Peripheral's interrupt priority */
    unsigned int intPriority;
} SensorsTiva_HWAttrs;
