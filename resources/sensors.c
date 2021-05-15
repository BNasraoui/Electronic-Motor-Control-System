#include "sensors.h"
#include <math.h>

/*
 * This is the main sensor task that will spin and
 * check sensor data against criteria
 */

void InitI2C_opt3001(I2C_Handle* opt3001) {
    uint8_t txBuffer[3];
    I2C_Params      opt3001Params;
    I2C_Transaction i2cTransaction;

    /* Create I2C for usage */
    I2C_Params_init(&opt3001Params);
    opt3001Params.bitRate = I2C_400kHz;
    opt3001 = I2C_open(0, &opt3001Params);
    if (opt3001 == NULL) {
        System_abort("Error Initializing Opt3001 I2C\n");
    } else {
    System_printf("IOpt3001 I2C Initialized!\n");
    }

    txBuffer[0] = REG_CONFIGURATION;
    txBuffer[1] = 0xC4;
    txBuffer[2] = 0x10;

    i2cTransaction.slaveAddress = OPT3001_I2C_ADDRESS;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 3;
    i2cTransaction.readBuf = NULL;
    i2cTransaction.readCount = 0;

    if (!(I2C_transfer(opt3001, &i2cTransaction))) {
        System_abort("Bad I2C transfer!");
    }

//    GPIO_write(Board_LED1, Board_LED_ON);

    System_flush();
}

bool SensorOpt3001Read(I2C_Handle opt3001, uint16_t *rawData)
{
    bool success;
    uint16_t val;

    success = ReadI2C(opt3001, REG_CONFIGURATION, &val);

    if (success)
    {
        success = ReadI2C(opt3001, REG_RESULT, &val);
    }

    if (success)
    {
        // Swap bytes
        *rawData = (val << 8) | (val>>8 &0xFF);
    }
    else
    {
        //    sensorSetErrorData
    }

    return (success);
}

//BLAH
bool ReadI2C(I2C_Handle opt3001, uint8_t ui8Reg, uint16_t *data){

    I2C_Transaction i2cTransaction;
    uint8_t txBuffer[1];
    uint8_t rxBuffer[2];

    txBuffer[0] = ui8Reg;

    i2cTransaction.slaveAddress = OPT3001_I2C_ADDRESS;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 2;

    if (!I2C_transfer(opt3001, &i2cTransaction)) {
        System_abort("Bad I2C Read transfer!");
    }
    data[0] = rxBuffer[0];
    data[1] = rxBuffer[1];
//    Task_sleep(500);
    return true;
}

void SensorOpt3001Convert(uint16_t rawData, float *convertedLux)
{
    uint16_t e, m;

    m = rawData & 0x0FFF;
    e = (rawData & 0xF000) >> 12;

    *convertedLux = m * (0.01 * exp2(e));
}


