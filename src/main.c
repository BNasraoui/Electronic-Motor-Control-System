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

/* Board Header file */
#include "Board.h"

void OPT3001Fxn()
{
    UInt gateKey;
    gateKey = GateHwi_enter(gateHwi);
    lightLimitReached = true;

    //Generate light limit event?
    //Call Swi to get lux value that triggered event???
    //I2C read doesn't like being called in Hwi because it's blocking??

    GateHwi_leave(gateHwi, gateKey);
}

void ADC0_Read() {
    uint32_t pui32ADC0Value[1];
    ADCIntClear(ADC0_BASE, ADC_SEQ);
    ADC0Sum = ADC0Sum - ADC0Buffer[ADC0BufferIndex];
    ADCSequenceDataGet(ADC0_BASE, ADC_SEQ , pui32ADC0Value);
    ADC0Buffer[ADC0BufferIndex] = pui32ADC0Value[0];
    Swi_post(swi0Handle);
}

void ADC0_FilterFxn() {
    ADC0Sum = ADC0Sum + ADC0Buffer[ADC0BufferIndex];
    ADC0BufferIndex =  (ADC0BufferIndex + 1) % ADC_BUFFER_SIZE;
    ADC0Avg = ADC0Sum / ADC_BUFFER_SIZE;
}

void ADC1_Read() {
    uint32_t pui32ADC1Value[1];
    ADCIntClear(ADC1_BASE, ADC_SEQ);
    ADC1Sum = ADC1Sum - ADC1Buffer[ADC1BufferIndex];
    ADCSequenceDataGet(ADC1_BASE, ADC_SEQ , pui32ADC1Value);
    ADC1Buffer[ADC1BufferIndex] = pui32ADC1Value[0];
    Swi_post(swi1Handle);
}

void ADC1_FilterFxn() {
    ADC1Sum = ADC1Sum + ADC1Buffer[ADC1BufferIndex];
    ADC1BufferIndex =  (ADC1BufferIndex + 1) % ADC_BUFFER_SIZE;
    ADC1Avg = ADC1Sum / ADC_BUFFER_SIZE;
}

void ReadSensorsFxn() {
    UInt gateKey;
    uint16_t rawData;
    uint16_t accelData[3];
    bool success = false;
    float luxFloat;

    InitI2C_opt3001();
    InitI2C_BMI160();
    InitADC0_CurrentSense();
    InitADC1_CurrentSense();

    while (1) {
        GPIO_write(Board_LED1, Board_LED_ON);

        gateKey = GateHwi_enter(gateHwi);
        success = SensorOpt3001Read(opt3001, &rawData);
        if (success) {
           SensorOpt3001Convert(rawData, &luxFloat);
           System_printf("LUX: %d\n", (int)luxFloat);
        }

        GateHwi_leave(gateHwi, gateKey);

        success = SensorBMI160_GetAccelData(accelData);
        System_printf("X: %d\t Y: %d\tZ: %d\n", accelData[0], accelData[1], accelData[2]);
        System_flush();

        GPIO_write(Board_LED1, Board_LED_OFF);
    }
}

/*
 *  ======== main ========
 */
int main(void)
{
    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();
    Board_initI2C();

    //This is the custom driver implementation init function
    //That will use Fxn table etc
    Board_initSensors();

    InitialiseTasks();

    GPIO_setCallback(Board_OPT3001, (GPIO_CallbackFxn)OPT3001Fxn);

    GPIO_enableInt(Board_OPT3001);

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
