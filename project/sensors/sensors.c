#include <GUI/homescreen/GUI_homescreen.h>
#include "sensors.h"
#include "bmi160/bmi160.h"
#include "opt3001/opt3001.h"
#include "GUI/graphing/GUI_graph.h"

//*************************** SWI/HWIS ******************************************
void OPT3001_ClockHandlerFxn() {
    //Event_post(sensors_eventHandle, NEW_OPT3001_DATA);
    gateKey = GateMutex_enter(gateHwi);
    BufferReadI2C_OPT3001(OPT3001_SLAVE_ADDRESS, REG_RESULT);
    Clock_start(opt3001_ClockHandler);
}

void ADC_ClockHandlerFxn() {
    ADCProcessorTrigger(ADC1_BASE, ADC_SEQ);
    Clock_start(adc_ClockHandler);
}

// fired when GPIO pin goes low due to new bmi160 data ready
void BMI160Fxn() {
    //Event_post(sensors_eventHandle, NEW_ACCEL_DATA);
    gateKey = GateMutex_enter(gateHwi);
    BufferReadI2C_BMI160(BMI160_SLAVE_ADDRESS, BMI160_RA_ACCEL_X_L);
}

// fired when low/high light event interrupt detected on GPIO pin
void OPT3001Fxn() {
    gateKey = GateMutex_enter(gateHwi);
    BufferReadI2C_OPT3001(OPT3001_SLAVE_ADDRESS, REG_CONFIGURATION);
    //Event_post(sensors_eventHandle, LOW_HIGH_LIGHT_EVENT);
}

void BufferReadI2C_OPT3001(uint8_t slaveAddress, uint8_t ui8Reg)
{
    //Use global reg as we use it to check what reg we are reading
    txBuffer_OPT[0] = ui8Reg;

    i2cTransaction.slaveAddress = slaveAddress;
    i2cTransaction.writeBuf = txBuffer_OPT;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer_OPT;
    i2cTransaction.readCount = 2;
    I2C_transfer(i2cHandle, &i2cTransaction);
}

void BufferReadI2C_BMI160(uint8_t slaveAddress, uint8_t ui8Reg)
{
    uint8_t txBuffer_BMI[1];
    txBuffer_BMI[0] = ui8Reg;

    i2cTransaction.slaveAddress = slaveAddress;
    i2cTransaction.writeBuf = txBuffer_BMI;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer_BMI;
    i2cTransaction.readCount = 6;
    I2C_transfer(i2cHandle, &i2cTransaction);
}

void I2C_Callback(I2C_Handle handle, I2C_Transaction *i2cTransaction, bool result) {
    //We must protect the integrity of the I2C transaction
    //If it's used whilst we are processing then issues can arise
    if(result) {
        if(i2cTransaction->slaveAddress == BMI160_SLAVE_ADDRESS){
            //shift data from buffers to form raw accel
            rawAccel.x = (((int16_t)rxBuffer_BMI[1])  << 8) | rxBuffer_BMI[0];
            rawAccel.y = (((int16_t)rxBuffer_BMI[3])  << 8) | rxBuffer_BMI[2];
            rawAccel.z = (((int16_t)rxBuffer_BMI[5])  << 8) | rxBuffer_BMI[4];
            GateMutex_leave(gateHwi, gateKey);
            Swi_post(swiHandle_accelDataProc);
            return;
        }
        else if(i2cTransaction->slaveAddress == OPT3001_SLAVE_ADDRESS) {
            if(txBuffer_OPT[0] == REG_RESULT) {
                rawData = SwapBytes(rxBuffer_OPT);
                GateMutex_leave(gateHwi, gateKey);
                Swi_post(swiHandle_LuxDataProc);
                return;
            }
            else if(txBuffer_OPT[0] == REG_CONFIGURATION) {
                bool lowLightEventOccured = CheckLowLightEventOccured(rxBuffer_OPT);
                GateMutex_leave(gateHwi, gateKey);
                if(lowLightEventOccured && headLightState == OFF) {
                    Event_post(sensors_eventHandle, LOW_HIGH_LIGHT_EVENT);
                }
                return;
            }
        }
    }
}

//*************************** INITIALISATION **************************************
void InitSensorDriver() {
    Watchdog_Params watchDogParams;
    Error_init(&eb);

    InitInterrupts();

    /* Create I2C for usage */
    //I2C_Params i2cParams;
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_100kHz;
    i2cParams.transferMode = I2C_MODE_BLOCKING;
    i2cHandle = I2C_open(0, &i2cParams);
    if (i2cHandle == NULL) {
        System_abort("Error Initializing I2C Handle\n");
    }

    Clock_Params_init(&clockParams);
    clockParams.period = CLOCK_PERIOD_2HZ;
    clockParams.startFlag = FALSE;
    opt3001_ClockHandler = Clock_create(OPT3001_ClockHandlerFxn, CLOCK_TIMEOUT_MS, &clockParams, &eb);
    if (opt3001_ClockHandler == NULL) {
     System_abort("opt3001 clock handle create failed");
    }

    clockParams.period = CLOCK_PERIOD_150HZ;
    adc_ClockHandler = Clock_create(ADC_ClockHandlerFxn, CLOCK_TIMEOUT_MS, &clockParams, &eb);
    if (adc_ClockHandler == NULL) {
     System_abort("adc clock handle create failed");
    }

    clockParams.period = CLOCK_PERIOD_1HZ;
    watchDog_ClockHandler = Clock_create(TaskStatusCheck, CLOCK_TIMEOUT_MS, &clockParams, &eb);
    if (watchDog_ClockHandler == NULL) {
     System_abort("watchdog clock create failed");
    }

    clockParams.period = CLOCK_PERIOD_1HZ;
    widgetQueue_ClockHandler = Clock_create(UpdateWidgetQueue, CLOCK_TIMEOUT_MS, &clockParams, &eb);
    if (widgetQueue_ClockHandler == NULL) {
     System_abort("watchdog clock create failed");
    }

    //Create Hwi Gate Mutex
    GateMutex_Params gHwiprms;
    GateMutex_Params_init(&gHwiprms);
    gateHwi = GateMutex_create(&gHwiprms, NULL);
    if (gateHwi == NULL) {
        System_abort("Gate Hwi create failed");
    }

    Watchdog_Params_init(&watchDogParams);
    watchDogParams.resetMode = Watchdog_RESET_OFF;
    watchDogParams.callbackFxn = WatchDogBite;
    watchDogParams.resetMode = Watchdog_RESET_OFF;
    watchDogHandle = Watchdog_open(EK_TM4C1294XL_WATCHDOG0, &watchDogParams);
    if (!watchDogHandle) {
        System_printf("Watchdog did not open");
    }
}

void InitInterrupts() {

    Swi_Params_init(&swiParams);
    swiParams.priority = 1;
    swiParams.trigger = 0;
    Swi_construct(&swi1Struct, (Swi_FuncPtr)ADC1_FilterFxn, &swiParams, NULL);
    swiHandle_ADC1DataProc = Swi_handle(&swi1Struct);
    if (swiHandle_ADC1DataProc == NULL) {
     System_abort("SWI 1 ADC1 filter create failed");
    }

    Swi_construct(&swi2Struct, (Swi_FuncPtr)ProcessAccelDataFxn, &swiParams, NULL);
    swiHandle_accelDataProc = Swi_handle(&swi2Struct);
    if (swiHandle_accelDataProc == NULL) {
     System_abort("SWI 2 process accel data create failed");
    }

    Swi_construct(&swi3Struct, (Swi_FuncPtr)ProcessLuxDataFxn, &swiParams, NULL);
    swiHandle_LuxDataProc = Swi_handle(&swi3Struct);
    if (swiHandle_LuxDataProc == NULL) {
     System_abort("SWI 3 process lux data create failed");
    }

    Hwi_Params_init(&hwiParams);
    hwiParams.priority = 0;
    hwi_ADC1 = Hwi_create(ADC1_SEQ1_VEC_NUM, (Hwi_FuncPtr)ADC1_Read, &hwiParams, NULL);
    if (hwi_ADC1 == NULL) {
     System_abort("ADC1 Hwi create failed");
    }
}

void ProcessSensorEvents() {
    static float delay = 0;
    UInt events;
    UInt gateKey;

    events = Event_pend(sensors_eventHandle, Event_Id_NONE, (Event_Id_00 + Event_Id_01 + Event_Id_02 + Event_Id_03 + Event_Id_04 + Event_Id_14), BIOS_WAIT_FOREVER);

    if(events & NEW_OPT3001_DATA) {

        //float lightLevel = GetLightLevel();

        if(headLightState == ON && luxValueFilt.avg > NIGHTTIME_LUX_VAL) {
            onDayNightChange(TURN_HEADLIGHTS_OFF);
            headLightState = OFF;
        }

        if (graphTypeActive == GRAPH_TYPE_LIGHT) {
            if (graphLagStart == 0) graphLagStart = Clock_getTicks();
            Event_post(GU_eventHandle, EVENT_GRAPH_LIGHT);
        }
    }

    if(events & NEW_ACCEL_DATA) {
        //GetAccelData();
        float absoluteAccel = CalcAbsoluteAccel();

        float accelLimit = 100; //for testing only
        if(absoluteAccel > accelLimit) {
            //Fire e-stop event
        }

        if (graphTypeActive == GRAPH_TYPE_ACCEL) {
            ++delay;
            if (delay >= 8) {
                delay = 0;
                if (graphLagStart == 0) graphLagStart = Clock_getTicks();
                Event_post(GU_eventHandle, EVENT_GRAPH_ACCEL);
            }
        }
    }

    if(events & LOW_HIGH_LIGHT_EVENT) {
        //only turn on the headlights if our filtered data
        //tells us it's nightime
        if(luxValueFilt.avg < NIGHTTIME_LUX_VAL) {
            onDayNightChange(TURN_HEADLIGHTS_ON);
            headLightState = ON;
        }
    }

    if(events & NEW_ADC0_DATA) {
        //Check if limit exceeded, respond accordingly

        //Update display
        //System_printf("ADC0: %f\n", ADC0Window.avg);
    }

    if(events & NEW_ADC1_DATA) {
        //Check if limit exceeded, respon accordingly

        if (graphTypeActive == GRAPH_TYPE_CURR) {
            if (graphLagStart == 0) graphLagStart = Clock_getTicks();
            Event_post(GU_eventHandle, EVENT_GRAPH_CURR);
        }

        //Update display
        //System_printf("ADC1: %f\n", ADC1Window.avg);
    }

    if(events & KICK_DOG) {
        //System_printf("Setting bit to tell watchdog that this task is ok");
        gateKey = GateHwi_enter(gateHwi);
        watchDogCheck = watchDogCheck | WATCHDOG_CHECKIN_SENSOR;
        //For testing
        watchDogCheck = watchDogCheck | WATCHDOG_CHECKIN_MOTOR;
        watchDogCheck = watchDogCheck | WATCHDOG_CHECKIN_GUI;
        GateHwi_leave(gateHwi, gateKey);
    }
}

float GetLightLevel() {
    GetLuxValue(&rawData);
    Swi_post(swiHandle_LuxDataProc);
    return luxValueFilt.avg;
}

void GetAccelData() {
    GetAccelData_BMI160(&accelX, &accelY, &accelZ);
    Swi_post(swiHandle_accelDataProc);
}

void InitADC1_CurrentSense() {
    ADC0Window.index = 0;
    ADC0Window.sum = 0;
    ADC0Window.avg = 0;
    ADC0Window.voltage = 0;
    ADC0Window.current = 0;
    ADC0Window.power = 0;
    ADC0Window.startFilter = false;

    ADC1Window.index = 0;
    ADC1Window.sum = 0;
    ADC1Window.avg = 0;
    ADC1Window.voltage = 0;
    ADC1Window.current = 0;
    ADC1Window.power = 0;
    ADC1Window.startFilter = false;

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);

    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_7);
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);

    ADCSequenceConfigure(ADC1_BASE, ADC_SEQ , ADC_TRIGGER_PROCESSOR, 0);

    ADCSequenceStepConfigure(ADC1_BASE, ADC_SEQ, 0, ADC_CTL_CH0);
    ADCSequenceStepConfigure(ADC1_BASE, ADC_SEQ, 1, ADC_CTL_IE | ADC_CTL_CH4 | ADC_CTL_END);

    ADCSequenceEnable(ADC1_BASE, ADC_SEQ);
    ADCIntEnable(ADC1_BASE, ADC_SEQ);
    ADCIntClear(ADC1_BASE, ADC_SEQ);
    IntEnable(INT_ADC1SS1);
}

void ADC1_Read() {
    uint32_t pui32ADC1Value[2];

    ADCIntClear(ADC1_BASE, ADC_SEQ);

    ADC1Window.sum = ADC1Window.sum - ADC1Window.data[ADC1Window.index];
    ADC0Window.sum = ADC0Window.sum - ADC0Window.data[ADC0Window.index];

    ADCSequenceDataGet(ADC1_BASE, ADC_SEQ , pui32ADC1Value);

    ADC1Window.data[ADC1Window.index] = pui32ADC1Value[0];
    ADC0Window.data[ADC0Window.index] = pui32ADC1Value[1];

    Swi_post(swiHandle_ADC1DataProc);
    //Swi_post(swiHandle_ADC0DataProc);
}

void ADC1_FilterFxn() {
    ADC1Window.sum = ADC1Window.sum + ADC1Window.data[ADC1Window.index];
    ADC0Window.sum = ADC0Window.sum + ADC0Window.data[ADC0Window.index];

    ADC1Window.index = (ADC1Window.index + 1) % WINDOW_SIZE;
    ADC0Window.index = (ADC0Window.index + 1) % WINDOW_SIZE;

    if(ADC1Window.startFilter) {
        ADC1Window.avg = (float)ADC1Window.sum / WINDOW_SIZE;
        ADC0Window.avg = (float)ADC0Window.sum / WINDOW_SIZE;

        ADC1Window.voltage = ADC1Window.avg * ADC_RESOLUTION;
        ADC0Window.voltage = ADC0Window.avg * ADC_RESOLUTION;

        ADC1Window.current = ADC1Window.voltage / SHUNT_R_VALUE;
        ADC0Window.current = ADC0Window.voltage / SHUNT_R_VALUE;

        ADC1Window.power = ADC1Window.voltage * ADC1Window.current;
        ADC1Window.power = ADC1Window.voltage * ADC1Window.current;

        Event_post(sensors_eventHandle, Event_Id_04);
    }

    if((ADC1Window.index + 1) == WINDOW_SIZE && ADC1Window.startFilter == false) {
        ADC1Window.startFilter = true;
    }
}

//void ADC0_FilterFxn() {
//    ADC0Window.sum = ADC0Window.sum + ADC0Window.data[ADC0Window.index];
//    ADC0Window.index = (ADC0Window.index + 1) % WINDOW_SIZE;
//
//    if(ADC0Window.startFilter) {
//        ADC0Window.avg = (float)ADC0Window.sum / WINDOW_SIZE;
//        ADC0Window.voltage = ADC1Window.avg / ADC_RESOLUTION;
//        ADC0Window.current = ADC0Window.voltage / SHUNT_R_VALUE;
//        ADC1Window.power = ADC1Window.voltage * ADC1Window.current;
//        Event_post(sensors_eventHandle, Event_Id_03);
//    }
//
//    if((ADC0Window.index + 1) == WINDOW_SIZE && ADC0Window.startFilter == false) {
//        ADC0Window.startFilter = true;
//    }
//}

//****************************HELPER FUNCTIONS********************************************
bool ReadHalfWordI2C(I2C_Handle i2cHandle, uint8_t slaveAddress, uint8_t ui8Reg, uint8_t* data) {

    I2C_Transaction i2cTransaction;
    uint8_t txBuffer[1];
    uint8_t rxBuffer[2];

    txBuffer[0] = ui8Reg;

    i2cTransaction.slaveAddress = slaveAddress;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 2;

    if (!I2C_transfer(i2cHandle, &i2cTransaction)) {
        System_printf("Bad I2C Read transfer!");
        System_flush();
    }
    data[0] = rxBuffer[0];
    data[1] = rxBuffer[1];
    return true;
}

bool ReadByteI2C(I2C_Handle i2cHandle, uint8_t slaveAddress, uint8_t ui8Reg, uint8_t *data) {
    I2C_Transaction i2cTransaction;
    uint8_t txBuffer[1];
    uint8_t rxBuffer[1];

    txBuffer[0] = ui8Reg;

    i2cTransaction.slaveAddress = slaveAddress;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;

    if (!I2C_transfer(i2cHandle, &i2cTransaction)) {
        System_printf("Bad I2C Read transfer!");
    }

    *data = rxBuffer[0];
    return true;
}

/*
 * Sets slave address to ui8Addr
 * Puts ui8Reg followed by two data bytes in *data and transfers
 * over i2c
 */
bool WriteHalfwordI2C(I2C_Handle i2cHandle, uint8_t slaveAddress, uint8_t ui8Reg, uint8_t *data) {
    I2C_Transaction i2cTransaction;
    uint8_t txBuffer[3];
    uint8_t rxBuffer[2];

    txBuffer[0] = ui8Reg;
    txBuffer[1] = data[0];
    txBuffer[2] = data[1];

    i2cTransaction.slaveAddress = slaveAddress;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 3;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;

    if (!I2C_transfer(i2cHandle, &i2cTransaction)) {
        System_printf("Bad I2C Write transfer!");
    }

    return true;
}
/*
 * Sets slave address to ui8Addr
 * Puts ui8Reg followed by two data bytes in *data and transfers
 * over i2c
 */
bool WriteByteI2C(I2C_Handle i2cHandle, uint8_t slaveAddress, uint8_t ui8Reg, uint8_t data) {
    I2C_Transaction i2cTransaction;
    uint8_t txBuffer[2];
    uint8_t rxBuffer[2];

    txBuffer[0] = ui8Reg;
    txBuffer[1] = data;

    i2cTransaction.slaveAddress = slaveAddress;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;

    if (!I2C_transfer(i2cHandle, &i2cTransaction)) {
        System_printf("Bad I2C Write transfer!");
    }

    return true;
}

//********************** CODE GRAVEYARD ****************************************
