/* XDCtools Header files */
#include <xdc/std.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>

/* TI-RTOS Header files */
#include <ti/drivers/EMAC.h>
#include <ti/drivers/GPIO.h>

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

void ReadSensorsFxn() {
    UInt events;

    InitI2C_OPT3001();
    InitI2C_BMI160();
    InitADC0_CurrentSense();
    InitADC1_CurrentSense();

    // re-init i2c in callback mode for periodic sensor reading
    I2C_close(i2cHandle);
    i2cParams.transferMode = I2C_MODE_CALLBACK;
    i2cParams.transferCallbackFxn = I2C_Callback;
    i2cHandle = I2C_open(0, &i2cParams);
    if (i2cHandle == NULL) {
        System_abort("Error Initializing I2C Handle\n");
    }

    // enable Hwi for BMI160 and OPT3001
    GPIO_setCallback(Board_BMI160, (GPIO_CallbackFxn)BMI160Fxn);
    GPIO_setCallback(Board_OPT3001, (GPIO_CallbackFxn)OPT3001Fxn);
    GPIO_enableInt(Board_BMI160);
    GPIO_enableInt(Board_OPT3001);

    //Start the timing clocks used to periodically trigger opt3001 and bmi160 reads
    Clock_start(clockHandler);
    Clock_start(clockHandler2);

    while (1) {
        GPIO_write(Board_LED1, Board_LED_ON);
        events = Event_pend(eventHandler, Event_Id_NONE, (Event_Id_00 + Event_Id_01 + Event_Id_02 + Event_Id_03 + Event_Id_04), BIOS_WAIT_FOREVER);

        if(events & NEW_OPT3001_DATA) {
            //GetLuxValue_OPT3001(&rawData);
            //Swi_post(swi3Handle);
            //GPIO_write(Board_LED0, Board_LED_OFF);
            System_printf("LUX: %f\n", luxValueFilt.avg);
        }
        if(events & NEW_ACCEL_DATA) {
            //GetAccelData_BMI160(&accelX, &accelY, &accelZ);
            //Swi_post(swi2Handle);
            System_printf("X: %f\t Y: %f\t Z: %f\n", accelXFilt.G, accelYFilt.G, accelZFilt.G);
        }

        if(events & LOW_HIGH_LIGHT_EVENT) {
            //TURN ON/OFF HEADLIGHTS
            System_printf("LOW/HIGH light even\n");
        }
        if(events & NEW_ADC0_DATA) {
            //Check if limit exceeded, respon accordingly

            //Update display
            System_printf("ADC0: %f\n", ADC0Window.avg);
        }
        if(events & NEW_ADC1_DATA) {
            //Check if limit exceeded, respon accordingly

            //Update display
            System_printf("ADC1: %f\n", ADC1Window.avg);
        }

        System_flush();
        GPIO_write(Board_LED1, Board_LED_OFF);
    }
}

int main(void)
{
    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();
    Board_initI2C();

    //Board_initSensors();
    InitSensorDriver();

    /* Turn on user LED  */
    GPIO_write(Board_LED0, Board_LED_ON);

    /* Start BIOS */
    BIOS_start();

    return (0);
}
