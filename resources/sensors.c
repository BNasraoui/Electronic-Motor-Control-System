#include "sensors.h"
#include "bmi160.h"

//*************************** SWI/HWIS **************************************
void OPT3001_ClockHandlerFxn() {
    //Event_post(eventHandler, Event_Id_01);
    BufferReadI2C_OPT3001(OPT3001_SLAVE_ADDRESS, REG_CONFIGURATION);
    BufferReadI2C_OPT3001(OPT3001_SLAVE_ADDRESS, REG_RESULT);
    Clock_start(opt3001_ClockHandler);
}

void ADC_ClockHandlerFxn() {
    ADCProcessorTrigger(ADC0_BASE, ADC_SEQ);
    ADCProcessorTrigger(ADC1_BASE, ADC_SEQ);
    Clock_start(adc_ClockHandler);
}

void BMI160Fxn() {
    //Event_post(eventHandler, Event_Id_02);
    BufferReadI2C_BMI160(BMI160_SLAVE_ADDRESS, BMI160_RA_ACCEL_X_L);
}

void OPT3001Fxn()
{
    Event_post(eventHandler, Event_Id_00);
}

void watchDogBite() {

    System_printf("watchdog not happy");
    System_flush();
    Clock_start(adc_ClockHandler);
    //Post events to each task
    //Event_post(eventHandler, Event_Id_00);
}

//*************************** INITIALISATION **************************************
void InitSensorDriver() {
    Watchdog_Params watchDogParams;
    Error_init(&eb);

    InitTasks();

    eventHandler = Event_create(NULL, &eb);
    if(eventHandler == NULL) {
        System_abort("Sensors event handle create failed");
    }

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
    watchDog_ClockHandler = Clock_create(ADC_ClockHandlerFxn, CLOCK_TIMEOUT_MS, &clockParams, &eb);
    if (watchDog_ClockHandler == NULL) {
     System_abort("watchdog clock create failed");
    }

    //Create Hwi Gate Mutex
    GateHwi_Params_init(&gHwiprms);
    gateHwi = GateHwi_create(&gHwiprms, NULL);
    if (gateHwi == NULL) {
        System_abort("Gate Hwi create failed");
    }

    Watchdog_Params_init(&watchDogParams);
    watchDogParams.resetMode = Watchdog_RESET_OFF;
    watchDogParams.callbackFxn = watchDogBite;
    watchDogParams.resetMode = Watchdog_RESET_OFF;
    watchDogHandle = Watchdog_open(EK_TM4C1294XL_WATCHDOG0, &watchDogParams);
    if (!watchDogHandle) {
        System_printf("Watchdog did not open");
    }
}

void InitTasks() {
    Task_Params taskParams;
    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task0Stack;
    taskParams.instance->name = "sensorTask";
    taskParams.priority = 0;
    Task_construct(&task0Struct, (Task_FuncPtr)ReadSensorsFxn, &taskParams, NULL);

    Swi_Params_init(&swiParams);
    swiParams.priority = 1;
    swiParams.trigger = 0;
    Swi_construct(&swi0Struct, (Swi_FuncPtr)ADC0_FilterFxn, &swiParams, NULL);
    swiHandle_ADC0DataProc = Swi_handle(&swi0Struct);
    if (swiHandle_ADC0DataProc == NULL) {
     System_abort("SWI 0 ADC0 filter create failed");
    }

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
    hwi_ADC0 = Hwi_create(ADC0_SEQ1_VEC_NUM, (Hwi_FuncPtr)ADC0_Read, &hwiParams, NULL);
    if (hwi_ADC0 == NULL) {
     System_abort("ADC0 Hwi create failed");
    }

    hwi_ADC1 = Hwi_create(ADC1_SEQ1_VEC_NUM, (Hwi_FuncPtr)ADC1_Read, &hwiParams, NULL);
    if (hwi_ADC1 == NULL) {
     System_abort("ADC1 Hwi create failed");
    }
}

//*************************** ACCELEROMETER BMI160 **************************************
void InitI2C_BMI160() {
    uint8_t val;
    uint8_t data;

    accelXFilt.index = 0;
    accelXFilt.sum = 0;
    accelXFilt.avg = 0;
    accelXFilt.startFilter = false;

    accelYFilt.index = 0;
    accelYFilt.sum = 0;
    accelYFilt.avg = 0;
    accelXFilt.startFilter = false;

    accelZFilt.index = 0;
    accelZFilt.sum = 0;
    accelZFilt.avg = 0;
    accelXFilt.startFilter = false;

    ReadByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMI160_RA_CHIP_ID, &data);
    if(data != BMI160_ID) {
        System_printf("Device ID: %d, should be = 209\n", data);
        System_flush();
        System_abort("Wrong bmi160 device ID returned - ABORTING");
    }

    /* Issue a soft-reset to bring the device into a clean state */
    val =  BMI160_CMD_SOFT_RESET;
    WriteByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMI160_RA_CMD, val);
    Task_sleep(10);

    /* Power up the accelerometer */
    val =  BMI160_CMD_ACC_MODE_NORMAL;
    WriteByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMI160_RA_CMD, val);
    Task_sleep(5);

    //Set the bandwidth for accell/gyro to normal and set 100Hz for both
    val =  BMI160_ACCEL_RATE_200HZ;
    WriteByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMI160_FOC_CONF_DEFAULT, val);

    //Set the full scale range to 2g
    val =  BMI160_ACCEL_RANGE_2G;
    WriteByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMI160_RA_ACCEL_RANGE, val);

    //Interrupt raised when data ready
    val =  BMI160_INT1EN_DATAREADY;
    WriteByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMI160_RA_INT_EN_1, val);

    //Map FIFO full interrupt type to int1
    val =  BMI160_INT1_MAP_DATAREADY;
    WriteByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMI160_RA_INT_MAP_1, val);

    //Set latching mode
    val =  BMI160_LATCH_MODE_NONE;
    WriteByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMI160_RA_INT_LATCH, val);

    //Set pin direction as output, select open-drain
    val =  BMI160_DIR_OUTPUT | BMI160_DIR_OPENDRAIN;
    WriteByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMI160_RA_INT_OUT_CTRL, val);

    #ifdef DEBUG_MODE
        //check error status of CHIP
        ReadByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMM150_OPMODE_REG_DEFAULT, &data);
        System_printf("Error code: %d\n", data);

        //check PMU register to make sure accel powered up
        ReadByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMI160_RA_PMU_STATUS, &data);
        System_printf("PMU status: %d\n", data);

        //Check the interrupt status to make sure correct interrupt raised
        ReadByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMI160_RA_INT_STATUS_1, &data);
        System_printf("Int status: %d\n", data);
        System_flush();
    #endif
}


bool GetAccelData_BMI160(int16_t *accelX, int16_t *accelY, int16_t *accelZ) {
    I2C_Transaction i2cTransaction;
    uint8_t rxBuffer[6];
    uint8_t txBuffer[1];

    txBuffer[0] = BMI160_RA_ACCEL_X_L;

    i2cTransaction.slaveAddress = BMI160_SLAVE_ADDRESS;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 6;
    I2C_transfer(i2cHandle, &i2cTransaction);

    *accelX = (((int16_t)rxBuffer[1])  << 8) | rxBuffer[0];
    *accelY = (((int16_t)rxBuffer[3])  << 8) | rxBuffer[2];
    *accelZ = (((int16_t)rxBuffer[5])  << 8) | rxBuffer[4];

    return true;
}

void ProcessAccelDataFxn() {
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

    if(accelXFilt.startFilter) {
        accelXFilt.avg = (float)accelXFilt.sum / WINDOW_SIZE;
        accelYFilt.avg = (float)accelYFilt.sum / WINDOW_SIZE;
        accelZFilt.avg = (float)accelZFilt.sum / WINDOW_SIZE;
        ConvertRawAccelToGs();
    }

    //All indexes incremented at same time so only check one filter
    if((accelXFilt.index + 1) == WINDOW_SIZE && accelXFilt.startFilter == false) {
        accelXFilt.startFilter = true;
    }

    Event_post(eventHandler, Event_Id_02);
}

void ConvertRawAccelToGs() {
    //convert to Gs
    float scale_2g = 4000.0/BMI160_2G_RANGE;
    float accelX_mG = accelXFilt.avg * scale_2g;
    float accelY_mG = accelYFilt.avg * scale_2g;
    float accelZ_mG = accelZFilt.avg * scale_2g;
    accelXFilt.G = accelX_mG/1000;
    accelYFilt.G = accelY_mG/1000;
    accelZFilt.G = accelZ_mG/1000;
}

//*************************** LIGHT SENSING OPT3001 **************************************
void InitI2C_OPT3001() {
    luxValueFilt.index = 0;
    luxValueFilt.sum = 0;
    luxValueFilt.avg = 0;
    luxValueFilt.startFilter = false;
    uint8_t data;
    uint16_t val;

    //Read device ID
    bool success = ReadByteI2C(i2cHandle, OPT3001_SLAVE_ADDRESS, REG_DEVICE_ID, &data);
    while(data != 48) {
        System_printf("I2C FAIL\t trying again in 5ms\n");
        System_flush();
        Task_sleep(5);
        bool success = ReadByteI2C(i2cHandle, OPT3001_SLAVE_ADDRESS, REG_DEVICE_ID, &data);
    }

    val =  CONFIG_VAL;
    WriteHalfwordI2C(i2cHandle, OPT3001_SLAVE_ADDRESS, REG_CONFIGURATION, (uint8_t*)&val);

    //Configure the default high/low limits
    SetLowLimit_OPT3001(40.95);
    SetHighLimit_OPT3001(2620.8);

    IntEnable(INT_GPIOP2);
}

bool GetLuxValue_OPT3001(uint16_t *rawData) {
    bool success;
    uint16_t val;

    success = ReadHalfWordI2C(i2cHandle, OPT3001_SLAVE_ADDRESS, REG_CONFIGURATION, (uint8_t*)&val);

    if (success) {
        success = ReadHalfWordI2C(i2cHandle, OPT3001_SLAVE_ADDRESS, REG_RESULT, (uint8_t*)&val);
    }

    if (success) {
        // Swap bytes
        *rawData = (val << 8) | (val>>8 & 0xFF);
    }

    return (success);
}

void ProcessLuxDataFxn() {
    float lux;
    static bool led_state = false;

    SensorOpt3001Convert(rawData, &lux);
    luxValueFilt.sum = luxValueFilt.sum - luxValueFilt.data[luxValueFilt.index];
    luxValueFilt.data[luxValueFilt.index] = (uint16_t)lux;
    luxValueFilt.sum = luxValueFilt.sum + luxValueFilt.data[luxValueFilt.index];
    luxValueFilt.index = (luxValueFilt.index + 1) % WINDOW_SIZE;

    if(luxValueFilt.startFilter) {
        luxValueFilt.avg = (float)luxValueFilt.sum / WINDOW_SIZE;
    }

    if((luxValueFilt.index + 1) == WINDOW_SIZE  && luxValueFilt.startFilter == false) {
        luxValueFilt.startFilter = true;
    }
    led_state = !led_state;
    GPIO_write(Board_LED0, led_state);
    Event_post(eventHandler, Event_Id_01);
}

//***************************CURRENT SENSING**************************************
void InitADC0_CurrentSense() {
    ADC0Window.index = 0;
    ADC0Window.sum = 0;
    ADC0Window.avg = 0;
    ADC0Window.voltage = 0;
    ADC0Window.current = 0;
    ADC0Window.power = 0;
    ADC0Window.startFilter = false;

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    //Makes GPIO an INPUT and sets them to be ANALOG
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
    //uint32_t ui32Base, uint32_t ui32SequenceNum, uint32_t ui32Trigger, uint32_t
    ADCSequenceConfigure(ADC0_BASE, ADC_SEQ , ADC_TRIGGER_PROCESSOR, 0);

    //uint32_t ui32Base, uint32_t ui32SequenceNum, uint32_t ui32Step, uint32_t ui32Config
    ADCSequenceStepConfigure(ADC0_BASE, ADC_SEQ , ADC_STEP , ADC_CTL_IE | ADC_CTL_CH0 | ADC_CTL_END);

    //Enable everything
    ADCSequenceEnable(ADC0_BASE, ADC_SEQ);
    ADCIntEnable(ADC0_BASE, ADC_SEQ);
    ADCIntClear( ADC0_BASE, ADC_SEQ);
    IntEnable(INT_ADC0SS1);
}

void InitADC1_CurrentSense() {
    ADC1Window.index = 0;
    ADC1Window.sum = 0;
    ADC1Window.avg = 0;
    ADC1Window.voltage = 0;
    ADC1Window.current = 0;
    ADC1Window.power = 0;
    ADC1Window.startFilter = false;

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);

    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_7);

    ADCSequenceConfigure(ADC1_BASE, ADC_SEQ , ADC_TRIGGER_PROCESSOR, 0);

    ADCSequenceStepConfigure(ADC1_BASE, ADC_SEQ , ADC_STEP , ADC_CTL_IE | ADC_CTL_CH4 | ADC_CTL_END);

    ADCSequenceEnable(ADC1_BASE, ADC_SEQ);
    ADCIntEnable(ADC1_BASE, ADC_SEQ);
    ADCIntClear(ADC1_BASE, ADC_SEQ);
    IntEnable(INT_ADC1SS1);
}

void ADC0_Read() {
    uint32_t pui32ADC0Value[1];

    ADCIntClear(ADC0_BASE, ADC_SEQ);
    ADC0Window.sum = ADC0Window.sum - ADC0Window.data[ADC0Window.index];
    ADCSequenceDataGet(ADC0_BASE, ADC_SEQ, pui32ADC0Value);
    ADC0Window.data[ADC0Window.index] = pui32ADC0Value[0];
    Swi_post(swiHandle_ADC0DataProc);
}

void ADC0_FilterFxn() {
    ADC0Window.sum = ADC0Window.sum + ADC0Window.data[ADC0Window.index];
    ADC0Window.index = (ADC0Window.index + 1) % WINDOW_SIZE;

    if(ADC0Window.startFilter) {
        ADC0Window.avg = (float)ADC0Window.sum / WINDOW_SIZE;
        ADC0Window.voltage = ADC1Window.avg / ADC_RESOLUTION;
        ADC0Window.current = ADC0Window.voltage / SHUNT_R_VALUE;
        ADC1Window.power = ADC1Window.voltage * ADC1Window.current;
        Event_post(eventHandler, Event_Id_03);
    }

    if((ADC0Window.index + 1) == WINDOW_SIZE && ADC0Window.startFilter == false) {
        ADC0Window.startFilter = true;
    }
}

void ADC1_Read() {
    uint32_t pui32ADC1Value[1];

    ADCIntClear(ADC1_BASE, ADC_SEQ);
    ADC1Window.sum = ADC1Window.sum - ADC1Window.data[ADC1Window.index];
    ADCSequenceDataGet(ADC1_BASE, ADC_SEQ , pui32ADC1Value);
    ADC1Window.data[ADC1Window.index] = pui32ADC1Value[0];
    Swi_post(swiHandle_ADC1DataProc);
}

void ADC1_FilterFxn() {
    ADC1Window.sum = ADC1Window.sum + ADC1Window.data[ADC1Window.index];
    ADC1Window.index = (ADC1Window.index + 1) % WINDOW_SIZE;

    if(ADC1Window.startFilter) {
        ADC1Window.avg = (float)ADC1Window.sum / WINDOW_SIZE;
        ADC1Window.voltage = ADC1Window.avg / ADC_RESOLUTION;
        ADC1Window.current = ADC1Window.voltage / SHUNT_R_VALUE;
        ADC1Window.power = ADC1Window.voltage * ADC1Window.current;
        Event_post(eventHandler, Event_Id_04);
    }

    if((ADC1Window.index + 1) == WINDOW_SIZE && ADC1Window.startFilter == false) {
        ADC1Window.startFilter = true;
    }
}

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


void BufferReadI2C_OPT3001(uint8_t slaveAddress, uint8_t ui8Reg)
{
    UInt gateKey;
    txBuffer_OPT[0] = ui8Reg;

    gateKey = GateHwi_enter(gateHwi);

    i2cTransactionCallback.slaveAddress = slaveAddress;
    i2cTransactionCallback.writeBuf = txBuffer_OPT;
    i2cTransactionCallback.writeCount = 1;
    i2cTransactionCallback.readBuf = rxBuffer_OPT;
    i2cTransactionCallback.readCount = 2;
    I2C_transfer(i2cHandle, &i2cTransactionCallback);

    GateHwi_leave(gateHwi, gateKey);
}

void BufferReadI2C_BMI160(uint8_t slaveAddress, uint8_t ui8Reg)
{
    UInt gateKey;
    uint8_t txBuffer_BMI[1];
    txBuffer_BMI[0] = ui8Reg;

    gateKey = GateHwi_enter(gateHwi);

    i2cTransactionCallback.slaveAddress = slaveAddress;
    i2cTransactionCallback.writeBuf = txBuffer_BMI;
    i2cTransactionCallback.writeCount = 1;
    i2cTransactionCallback.readBuf = rxBuffer_BMI;
    i2cTransactionCallback.readCount = 6;
    I2C_transfer(i2cHandle, &i2cTransactionCallback);

    GateHwi_leave(gateHwi, gateKey);
}

void I2C_Callback(I2C_Handle handle, I2C_Transaction *i2cTransaction, bool result) {
    UInt gateKey;

    //We must protect the integrity of the I2C transaction
    //If it's used whilst we are processing then issues arise
    gateKey = GateHwi_enter(gateHwi);
    if(result) {
        if(i2cTransaction->slaveAddress == BMI160_SLAVE_ADDRESS){
            //shift data from buffers to accel axis
            accelX = (((int16_t)rxBuffer_BMI[1])  << 8) | rxBuffer_BMI[0];
            accelY = (((int16_t)rxBuffer_BMI[3])  << 8) | rxBuffer_BMI[2];
            accelZ = (((int16_t)rxBuffer_BMI[5])  << 8) | rxBuffer_BMI[4];
            Swi_post(swiHandle_accelDataProc);
        }
        else if(i2cTransaction->slaveAddress == OPT3001_SLAVE_ADDRESS) {
            //Only call Swi if we've requested result reg
            if(txBuffer_OPT[0] == REG_RESULT) {
                Swi_post(swiHandle_LuxDataProc);
            }
        }
    }
    else {
        System_printf("Bad i2c transaction");
    }
    System_flush();
    GateHwi_leave(gateHwi, gateKey);
}

void SensorOpt3001Convert(uint16_t rawData, float *convertedLux) {
    uint16_t e, m;

    m = rawData & 0x0FFF;
    e = (rawData & 0xF000) >> 12;

    *convertedLux = m * (0.01 * exp2(e));
}

void SetLowLimit_OPT3001(float val) {
    uint16_t reg = CalculateLimitReg(val);
    WriteHalfwordI2C(i2cHandle, OPT3001_SLAVE_ADDRESS, REG_LOW_LIMIT, (uint8_t*)&reg);
}

void SetHighLimit_OPT3001(float val) {
    uint16_t reg = CalculateLimitReg(val);
    WriteHalfwordI2C(i2cHandle, OPT3001_SLAVE_ADDRESS, REG_HIGH_LIMIT, (uint8_t*)&reg);
}

uint16_t CalculateLimitReg(float luxValue) {
    uint8_t E = 6;
    uint16_t result = luxValue / (0.01 * pow(2, E));
    unsigned char bytes[2];
    bytes[0] = (E << 4 & 0x000000F0) | (result >> 8 & 0x0000000F);
    bytes[1] = (result & 0x000000F0) | (result & 0x0000000F);
    uint16_t reg = bytes[1] << 8 | bytes[0];
    return reg;
}
