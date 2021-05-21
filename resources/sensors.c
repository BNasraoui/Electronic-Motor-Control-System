#include "sensors.h"
#include "bmi160.h"

void OPT3001Fxn()
{
    Event_post(eventHandler, Event_Id_00);
}

void clockHandlerFxn(UArg arg) {
    Event_post(eventHandler, Event_Id_01);
    Clock_start(clockHandler);
}

void EnableADCSequencers() {
    ADCProcessorTrigger(ADC0_BASE, ADC_SEQ);
    ADCProcessorTrigger(ADC1_BASE, ADC_SEQ);
    Clock_start(clockHandler2);
}

void BMI160Fxn() {
    Event_post(eventHandler, Event_Id_02);
}

void ProcessLuxDataFxn() {
    float lux;

    SensorOpt3001Convert(rawData, &lux);
    luxValueFilt.sum = luxValueFilt.sum - luxValueFilt.data[luxValueFilt.index];
    luxValueFilt.data[luxValueFilt.index] = (uint16_t)lux;
    luxValueFilt.sum = luxValueFilt.sum + luxValueFilt.data[luxValueFilt.index];
    luxValueFilt.index = (luxValueFilt.index + 1) % WINDOW_SIZE;
    luxValueFilt.avg = luxValueFilt.sum / WINDOW_SIZE;
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

    accelXFilt.avg = accelXFilt.sum / WINDOW_SIZE;
    accelYFilt.avg = accelYFilt.sum / WINDOW_SIZE;
    accelZFilt.avg = accelZFilt.sum / WINDOW_SIZE;
}

void ADC0_Read() {
    uint32_t pui32ADC0Value[1];

    ADCIntClear(ADC0_BASE, ADC_SEQ);
    ADC0Window.sum = ADC0Window.sum - ADC0Window.data[ADC0Window.index];
    ADCSequenceDataGet(ADC0_BASE, ADC_SEQ, pui32ADC0Value);
    ADC0Window.data[ADC0Window.index] = pui32ADC0Value[0];
    Swi_post(swi0Handle);
}

void ADC0_FilterFxn() {
    ADC0Window.sum = ADC0Window.sum + ADC0Window.data[ADC0Window.index];
    ADC0Window.index = (ADC0Window.index + 1) % WINDOW_SIZE;
    ADC0Window.avg = ADC0Window.sum / WINDOW_SIZE;
    ADC0Window.voltage = ADC1Window.avg / ADC_RESOLUTION;
    Event_post(eventHandler, Event_Id_03);
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
    ADC1Window.voltage = ADC1Window.avg / ADC_RESOLUTION;
    Event_post(eventHandler, Event_Id_04);
}

void InitSensorDriver() {

    Error_init(&eb);
    InitTasks();

    eventHandler = Event_create(NULL, &eb);
    if(eventHandler == NULL) {
        System_abort("Sensors event create failed");
    }

    /* Create I2C for usage */
    I2C_Params i2cParams;
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_100kHz;
    i2cParams.transferMode = I2C_MODE_BLOCKING;
    i2cHandle = I2C_open(0, &i2cParams);
    if (i2cHandle == NULL) {
        System_abort("Error Initializing I2C\n");
    }

    Clock_Params_init(&clockParams);
    clockParams.period = 50;
    clockParams.startFlag = FALSE;
    clockHandler = Clock_create(clockHandlerFxn, 50, &clockParams, &eb);
    if (clockHandler == NULL) {
     System_abort("Clock 1 create failed");
    }

    Clock_Params_init(&clockParams);
    clockParams.period = 3;
    clockParams.startFlag = FALSE;
    clockHandler2 = Clock_create(EnableADCSequencers, 3, &clockParams, &eb);
    if (clockHandler == NULL) {
     System_abort("Clock 2 create failed");
    }

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

}

void InitTasks() {
    /*
    Task_Params taskParams;

    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &sensorTaskStack;
    //taskParams.instance->name = "initI2C_opt3001";
    taskParams.priority = 1;
    Task_construct(&sensorTaskStruct, (Task_FuncPtr)ReadSensorsFxn, &taskParams, NULL);
    */

    Swi_Params_init(&swiParams);
    swiParams.priority = 1;
    swiParams.trigger = 0;
    Swi_construct(&swi0Struct, (Swi_FuncPtr)ADC0_FilterFxn, &swiParams, NULL);
    swi0Handle = Swi_handle(&swi0Struct);

    Swi_Params_init(&swiParams);
    swiParams.priority = 1;
    swiParams.trigger = 0;
    Swi_construct(&swi1Struct, (Swi_FuncPtr)ADC1_FilterFxn, &swiParams, NULL);
    swi1Handle = Swi_handle(&swi1Struct);

    Swi_Params_init(&swiParams);
    swiParams.priority = 1;
    swiParams.trigger = 0;
    Swi_construct(&swi2Struct, (Swi_FuncPtr)ProcessAccelDataFxn, &swiParams, NULL);
    swi2Handle = Swi_handle(&swi2Struct);

    Swi_Params_init(&swiParams);
    swiParams.priority = 1;
    swiParams.trigger = 0;
    Swi_construct(&swi3Struct, (Swi_FuncPtr)ProcessLuxDataFxn, &swiParams, NULL);
    swi3Handle = Swi_handle(&swi3Struct);

    Hwi_Params_init(&hwiParams);
    hwiParams.priority = 0;
    hwi_ADC0 = Hwi_create(ADC0_SEQ1_VEC_NUM, (Hwi_FuncPtr)ADC0_Read, &hwiParams, NULL);
    if (hwi_ADC0 == NULL) {
     System_abort("ADC0 Hwi create failed");
    }

    hwiParams.priority = 0;
    hwi_ADC1 = Hwi_create(ADC1_SEQ1_VEC_NUM, (Hwi_FuncPtr)ADC1_Read, &hwiParams, NULL);
    if (hwi_ADC1 == NULL) {
     System_abort("ADC1 Hwi create failed");
    }
}

void InitI2C_opt3001() {
    luxValueFilt.index = 0;
    luxValueFilt.sum = 0;
    luxValueFilt.avg = 0;
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

void InitI2C_BMI160() {
    accelXFilt.index = 0;
    accelXFilt.sum = 0;
    accelXFilt.avg = 0;

    accelYFilt.index = 0;
    accelYFilt.sum = 0;
    accelYFilt.avg = 0;

    accelZFilt.index = 0;
    accelZFilt.sum = 0;
    accelZFilt.avg = 0;
    uint8_t val;
    uint8_t data;

    ReadByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMI160_RA_CHIP_ID, &data);
    System_printf("Device ID: %d, should be = 209\n", data);
    System_flush();

    /* Issue a soft-reset to bring the device into a clean state */
    val =  BMI160_CMD_SOFT_RESET;
    WriteByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMI160_RA_CMD, val);
    Task_sleep(10);

    /* Power up the accelerometer */
    val =  BMI160_CMD_ACC_MODE_NORMAL;
    WriteByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMI160_RA_CMD, val);
    Task_sleep(5);

    //Set the bandwidth for accell/gyro to normal and set 100Hz for both
    val =  0x08;    //100Hz
    //val =  0x02;
    WriteByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMI160_FOC_CONF_DEFAULT, val);

    //Interrupt raised when data ready
    val =  0x10;
    WriteByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMI160_RA_INT_EN_1, val);

    //Map FIFO full interrupt type to int1
    val =  0x80;
    WriteByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMI160_RA_INT_MAP_1, val);

    //Set latching mode
    //val =  0x0D; //5ms latching
    val =  0x00; //no latching
    WriteByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMI160_RA_INT_LATCH, val);

    //Set pin direction as output, select push-pull
    val =  0x0C;
    WriteByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMI160_RA_INT_OUT_CTRL, val);

    //check error status of CHIP
    ReadByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMM150_OPMODE_REG_DEFAULT, &data);
    System_printf("Error code: %d\n", data);
    System_flush();

    //check PMU register to make sure accel powered up
    ReadByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMI160_RA_PMU_STATUS, &data);
    System_printf("PMU status: %d\n", data);
    System_flush();

    //Check the interrupt status to make sure correct interrupt raised
    ReadByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMI160_RA_INT_STATUS_1, &data);
    System_printf("Int status: %d\n", data);
    System_flush();
}

void InitADC0_CurrentSense() {
    ADC0Window.index = 0;
    ADC0Window.sum = 0;
    ADC0Window.avg = 0;
    ADC0Window.voltage = 0;
    ADC0Window.current = 0;
    ADC0Window.power = 0;

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

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);

    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_7);

    ADCSequenceConfigure(ADC1_BASE, ADC_SEQ , ADC_TRIGGER_PROCESSOR, 0);

    ADCSequenceStepConfigure(ADC1_BASE, ADC_SEQ , ADC_STEP , ADC_CTL_IE | ADC_CTL_CH4 | ADC_CTL_END);

    ADCSequenceEnable(ADC1_BASE, ADC_SEQ);
    ADCIntEnable(ADC1_BASE, ADC_SEQ);
    ADCIntClear(ADC1_BASE, ADC_SEQ);
    IntEnable(INT_ADC1SS1);
}

bool GetLuxValue_OPT3001(uint16_t *rawData)
{
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

bool ReadHalfWordI2C(I2C_Handle i2cHandle, uint8_t slaveAddress, uint8_t ui8Reg, uint8_t* data)
{

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

bool ReadByteI2C(I2C_Handle i2cHandle, uint8_t slaveAddress, uint8_t ui8Reg, uint8_t *data)
{

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
bool WriteHalfwordI2C(I2C_Handle i2cHandle, uint8_t slaveAddress, uint8_t ui8Reg, uint8_t *data)
{
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
bool WriteByteI2C(I2C_Handle i2cHandle, uint8_t slaveAddress, uint8_t ui8Reg, uint8_t data)
{
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

void SensorOpt3001Convert(uint16_t rawData, float *convertedLux)
{
    uint16_t e, m;

    m = rawData & 0x0FFF;
    e = (rawData & 0xF000) >> 12;

    *convertedLux = m * (0.01 * exp2(e));
}

void SetLowLimit_OPT3001(float val)
{
    uint16_t reg = CalculateLimitReg(val);
    WriteHalfwordI2C(i2cHandle, OPT3001_SLAVE_ADDRESS, REG_LOW_LIMIT, (uint8_t*)&reg);
}

void SetHighLimit_OPT3001(float val)
{
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

//********************************CODE GRAVEYARD***************************************
//    txBuffer[0] = REG_CONFIGURATION;
//    txBuffer[1] = 0xC4;
//    txBuffer[2] = 0x10;


//bool BufferReadI2C_BMI160(I2C_Handle handle, uint8_t slaveAddress, uint8_t ui8Reg, int numBytes)
//{
//    txBuffer_BMI[0] = ui8Reg;
//
//    i2cTransaction_BMI.slaveAddress = slaveAddress;
//    i2cTransaction_BMI.writeBuf = txBuffer_BMI;
//    i2cTransaction_BMI.writeCount = 1;
//    i2cTransaction_BMI.readBuf = rxBuffer_BMI;
//    i2cTransaction_BMI.readCount = numBytes;
//    I2C_transfer(handle, &i2cTransaction_BMI);
//    return true;
//}

//bool BufferReadI2C_OPT3001(I2C_Handle handle, uint8_t slaveAddress, uint8_t ui8Reg, int numBytes)
//{
//    txBuffer_OPT[0] = ui8Reg;
//    i2cTransaction_OPT.slaveAddress = slaveAddress;
//    i2cTransaction_OPT.writeBuf = txBuffer_OPT;
//    i2cTransaction_OPT.writeCount = 1;
//    i2cTransaction_OPT.readBuf = rxBuffer_OPT;
//    i2cTransaction_OPT.readCount = numBytes;
//    I2C_transfer(handle, &i2cTransaction_OPT);
//    return true;
//}


//**************************************CODE GRRAVEYARD********************************
//void I2C_Callback(I2C_Handle handle, I2C_Transaction *i2cTransaction, bool result) {
//    UInt gateKey;
//
//    //We must protect the integrity of the I2C transaction
//    //If it's used whilst we are processing then issues arise
//    gateKey = GateHwi_enter(gateHwi);
//    if(result) {
//        if(i2cTransaction->slaveAddress == BMI160_SLAVE_ADDRESS){
//            //Put data into x,y,z buffers
//            Swi_post(swi2Handle);
//        }
//        else if(i2cTransaction->slaveAddress == OPT3001_SLAVE_ADDRESS) {
//            if(txBuffer_OPT[0] == REG_RESULT) {
//                Swi_post(swi3Handle);
//            }
//            else {
//
//            }
//        }
//    }
//    else {
//        System_printf("Bad i2c transaction");
//
//    }
//    System_flush();
//    GateHwi_leave(gateHwi, gateKey);
//}

//    //Reinitialise the I2C interface in callback mode
//    I2C_close(i2cHandle);
//    i2cParams.transferMode = I2C_MODE_CALLBACK;
//    i2cParams.transferCallbackFxn = I2C_Callback;
//    i2cHandle = I2C_open(0, &i2cParams);
//    if (i2cHandle == NULL) {
//        System_abort("Error Initializing I2C in callback mode\n");
//    } else {
//        System_printf("I2C Initialized in callback mode\n");
//    }

////This is needed if we use i2c callback mode
////I2C_Transaction i2cTransaction_GLOBAL;
//typedef struct Sensors_Config      *Sensors_Handle;
//
//typedef struct SensorsTiva_Object {
//
//}SensorsTiva_Object;
//
///*!
// *  @brief  SENSORS Parameters
// *
// *  Params used to initialise the sensors driver
// *
// */
//typedef struct Sensors_Params_Params {
//   // Sensors_Params_TransferMode    transferMode; /*!< Blocking or Callback mode */
//   // Sensors_Params_CallbackFxn     transferCallbackFxn; /*!< Callback function pointer */
//   // Sensors_Params_BitRate         bitRate; /*!< I2C bus bit rate */
//  //  uintptr_t           custom;  /*!< Custom argument used by driver implementation */
//} Sensors_Params;
//
///*!
// *
// */
//typedef struct SENSORS_FxnTable {
//    /*! Function to close the specified peripheral */
//   // SENSORS_CloseFxn        closeFxn;
//
//    /*! Function to implementation specific control function */
//   // SENSORS_ControlFxn      controlFxn;
//
//    /*! Function to initialize the given data object */
//   // SENSORS_InitFxn         initFxn;
//
//    /*! Function to open the specified peripheral */
//    //SENSORS_OpenFxn         openFxn;
//
//    /*! Function to initiate a I2C data transfer */
//    //SENSORS_TransferFxn     transferFxn;
//} SENSORS_FxnTable;
//
///*!
// *
// *
// */
//typedef struct Sensors_Config {
//    /*! Pointer to a table of driver-specific implementations of I2C APIs */
//    I2C_FxnTable const *fxnTablePtr;
//
//    /*! Pointer to a driver specific data object */
//    void               *object;
//
//    /*! Pointer to a driver specific hardware attributes structure */
//    void         const *hwAttrs;
//} Sensors_Config;

//typedef struct SensorsTiva_HWAttrs {
//    /*! I2C Peripheral's base address */
//    unsigned int baseAddr;
//    /*! I2C Peripheral's interrupt vector */
//    unsigned int intNum;
//    /*! I2C Peripheral's interrupt priority */
//    unsigned int intPriority;
//} SensorsTiva_HWAttrs;
