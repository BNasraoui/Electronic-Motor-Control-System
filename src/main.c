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
#include <ti/drivers/EMAC.h>
#include <ti/drivers/GPIO.h>
// #include <ti/drivers/I2C.h>
// #include <ti/drivers/SDSPI.h>
// #include <ti/drivers/SPI.h>
// #include <ti/drivers/UART.h>
// #include <ti/drivers/USBMSCHFatFs.h>
// #include <ti/drivers/Watchdog.h>
// #include <ti/drivers/WiFi.h>

#include "driverlib/gpio.h"

/* Tiva C series macros header files */
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"

//Created libraries for sub-systems
#include "sensors.h"
#include "driverlib/adc.h"
#include "bmi160.h"

/* Board Header file */
#include "Board.h"

void BMI160Fxn()
{
    UInt gateKey;
    int numBytesToRead = 6;

    gateKey = GateHwi_enter(gateHwi);
    BufferReadI2C_BMI160(i2cHandle, BMI160_SLAVE_ADDRESS, BMI160_RA_ACCEL_X_L, numBytesToRead);
    GateHwi_leave(gateHwi, gateKey);
}

void OPT3001Fxn()
{
    UInt gateKey;
    int numBytesToRead = 2;

    gateKey = GateHwi_enter(gateHwi);
    //Clear the interrupt bit
    BufferReadI2C_OPT3001(i2cHandle, OPT3001_SLAVE_ADDRESS, REG_CONFIGURATION, numBytesToRead);

    //Need to Post low/high event

    GateHwi_leave(gateHwi, gateKey);
}

void I2C_Callback(I2C_Handle handle, I2C_Transaction *i2cTransaction, bool result) {
    UInt gateKey;

    //We must protect the integrity of the I2C transaction
    //If it's used whilst we are processing then issues arise
    gateKey = GateHwi_enter(gateHwi);
    if(result) {
        if(i2cTransaction->slaveAddress == BMI160_SLAVE_ADDRESS){
            //Put data into x,y,z buffers
            Swi_post(swi2Handle);
        }
        else if(i2cTransaction->slaveAddress == BMI160_SLAVE_ADDRESS) {
            if(txBuffer_OPT[0] == REG_RESULT) {
                uint16_t lux = i2cTransaction->readBuf;
                System_printf("LUX: %d\n", lux);
            }
        }
        System_flush();
    }
    GateHwi_leave(gateHwi, gateKey);
}

void ProcessAccelDataFxn() {
    int16_t accelX, accelY, accelZ;
    UInt gateKey;
    System_printf("BMI160 Swi trigger from I2C callback\n");

    gateKey = GateHwi_enter(gateHwi);

    accelX = (((int16_t)rxBuffer_BMI[1])  << 8) | rxBuffer_BMI[0];
    accelY = (((int16_t)rxBuffer_BMI[3])  << 8) | rxBuffer_BMI[2];
    accelZ = (((int16_t)rxBuffer_BMI[5])  << 8) | rxBuffer_BMI[4];

    GateHwi_leave(gateHwi, gateKey);

    accelXFilt.sum = accelXFilt.sum - accelXFilt.data[accelXFilt.index];
    accelYFilt.sum = accelYFilt.sum - accelYFilt.data[accelYFilt.index];
    accelZFilt.sum = accelZFilt.sum - accelZFilt.data[accelZFilt.index];

    accelXFilt.data[accelXFilt.index] = accelX;
    accelYFilt.data[accelYFilt.index] = accelY;
    accelZFilt.data[accelZFilt.index] = accelZ;

    accelXFilt.sum = accelXFilt.sum + accelXFilt.data[accelXFilt.index];
    accelYFilt.sum = accelYFilt.sum + accelYFilt.data[accelYFilt.index];
    accelZFilt.sum = accelZFilt.sum + accelZFilt.data[accelZFilt.index];

    accelXFilt.index = (accelXFilt.index + 1) % WINDOW_SIZE;
    accelYFilt.index = (accelYFilt.index + 1) % WINDOW_SIZE;
    accelZFilt.index = (accelZFilt.index + 1) % WINDOW_SIZE;

    accelXFilt.avg = accelXFilt.sum / WINDOW_SIZE;
    accelYFilt.avg = accelYFilt.sum / WINDOW_SIZE;
    accelZFilt.avg = accelZFilt.sum / WINDOW_SIZE;

    System_flush();
}

void ADC0_Read() {
    uint32_t pui32ADC0Value[1];
    ADCIntClear(ADC0_BASE, ADC_SEQ);
    ADC0Window.sum = ADC0Window.sum - ADC0Window.data[ADC0Window.index];
    ADCSequenceDataGet(ADC0_BASE, ADC_SEQ , pui32ADC0Value);
    ADC0Window.data[ADC0Window.index] = pui32ADC0Value[0];
    Swi_post(swi0Handle);
}

void ADC0_FilterFxn() {
    ADC0Window.sum = ADC0Window.sum + ADC0Window.data[ADC0Window.index];
    ADC0Window.index = (ADC0Window.index + 1) % WINDOW_SIZE;
    ADC0Window.avg = ADC0Window.sum / WINDOW_SIZE;
}

void ADC1_Read() {
    uint32_t pui32ADC1Value[1];
    ADCIntClear(ADC1_BASE, ADC_SEQ);
    ADC1Window.sum = ADC1Window.sum - ADC1Window.data[ADC1Window.index];
    ADCSequenceDataGet(ADC1_BASE, ADC_SEQ , pui32ADC1Value);
    ADC1Window.data[ADC1Window.index] = pui32ADC1Value[0];
    Swi_post(swi1Handle);
}

void ADC1_FilterFxn() {
    ADC1Window.sum = ADC1Window.sum + ADC1Window.data[ADC1Window.index];
    ADC1Window.index = (ADC1Window.index + 1) % WINDOW_SIZE;
    ADC1Window.avg = ADC1Window.sum / WINDOW_SIZE;
}

void ReadSensorsFxn() {
    UInt gateKey;
    uint16_t rawData;
    float luxFloat;

    InitI2C_opt3001();
    InitI2C_BMI160();
    InitADC0_CurrentSense();
    InitADC1_CurrentSense();

    //Reinitialise the I2C interface in callback mode
    I2C_close(i2cHandle);
    i2cParams.transferMode = I2C_MODE_CALLBACK;
    i2cParams.transferCallbackFxn = I2C_Callback;
    i2cHandle = I2C_open(0, &i2cParams);
    if (i2cHandle == NULL) {
        System_abort("Error Initializing I2C in callback mode\n");
    } else {
        System_printf("I2C Initialized in callback mode\n");
    }

    //enable Hwi for BMI160
    GPIO_setCallback(Board_BMI160, (GPIO_CallbackFxn)BMI160Fxn);
    GPIO_enableInt(Board_BMI160);
    //enable Hwi for Opt3001
    GPIO_setCallback(Board_OPT3001, (GPIO_CallbackFxn)OPT3001Fxn);
    GPIO_enableInt(Board_OPT3001);

    while (1) {
        GPIO_write(Board_LED1, Board_LED_ON);

        gateKey = GateHwi_enter(gateHwi);
        //Check filtered values to see if they exceed limits

        GateHwi_leave(gateHwi, gateKey);

        GPIO_write(Board_LED1, Board_LED_OFF);
    }
}

int main(void)
{
    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();
    Board_initI2C();

    //This is the custom driver implementation init function
    //That will use Fxn table etc
    //Board_initSensors();
    InitSensorDriver();

    //Create Hwi Gate Mutex
    GateHwi_Params_init(&gHwiprms);
    gateHwi = GateHwi_create(&gHwiprms, NULL);
    if (gateHwi == NULL) {
        System_abort("Gate Hwi create failed");
    }

    /* Turn on user LED  */
    GPIO_write(Board_LED0, Board_LED_ON);

    /* Start BIOS */
    BIOS_start();

    return (0);
}
