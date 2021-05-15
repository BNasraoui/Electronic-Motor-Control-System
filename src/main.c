/*
 * Copyright (c) 2015, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== empty_min.c ========
 */
/* XDCtools Header files */
#include <xdc/std.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

/* TI-RTOS Header files */
// #include <ti/drivers/EMAC.h>
#include <ti/drivers/GPIO.h>
// #include <ti/drivers/I2C.h>
// #include <ti/drivers/SDSPI.h>
// #include <ti/drivers/SPI.h>
// #include <ti/drivers/UART.h>
// #include <ti/drivers/USBMSCHFatFs.h>
// #include <ti/drivers/Watchdog.h>
// #include <ti/drivers/WiFi.h>

//Created libraries for sub-systems
#include "sensors.h"

/* Board Header file */
#include "Board.h"

#define TASKSTACKSIZE   512

Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];

void ReadSensorsFxn() {
    uint16_t rawData;
    bool success = 0;
    float luxFloat;
    I2C_Handle opt3001;

    //InitI2C_opt3001(opt3001);

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

    while (1) {
        GPIO_write(Board_LED1, Board_LED_ON);
        success = SensorOpt3001Read(opt3001, &rawData);

        if (success) {
           SensorOpt3001Convert(rawData, &luxFloat);
           int Lux = (int)luxFloat;

            if (luxFloat < 40)
               {
                   System_printf("Low Light Event: %d Lux\n", Lux);
               }
               else if (luxFloat > 2600)
               {
                   System_printf("High Light Event: %d Lux\n", Lux);
               }
               else
               {
                   System_printf("%d Lux\n", Lux);
               }

        }
        GPIO_write(Board_LED1, Board_LED_OFF);
        System_flush();
    }
}
/*
 *  ======== main ========
 */
int main(void)
{
    Task_Params taskParams;

    /* Call board init functions */
    Board_initGeneral();
    // Board_initEMAC();
    Board_initGPIO();
    Board_initI2C();
    // Board_initSDSPI();
    // Board_initSPI();
    // Board_initUART();
    // Board_initUSB(Board_USBDEVICE);
    // Board_initUSBMSCHFatFs();
    // Board_initWatchdog();
    // Board_initWiFi();

    Board_initSensors();

    /* Construct heartBeat Task  thread */
    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task0Stack;
    //taskParams.instance->name = "initI2C_opt3001";
    taskParams.priority = 1;
    Task_construct(&task0Struct, (Task_FuncPtr)ReadSensorsFxn, &taskParams, NULL);

    /* Turn on user LED  */
    GPIO_write(Board_LED0, Board_LED_ON);

    /* Start BIOS */
    BIOS_start();

    return (0);
}
