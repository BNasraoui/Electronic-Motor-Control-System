#include "sensors.h"
#include "bmi160.h"
#include <math.h>

void InitSensorDriver() {
    Task_Params taskParams;

    /* Create I2C for usage */
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_100kHz;
    i2cParams.transferMode = I2C_MODE_BLOCKING;
    i2cHandle = I2C_open(0, &i2cParams);

    if (i2cHandle == NULL) {
        System_abort("Error Initializing I2C\n");
    }

    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task0Stack;
    //taskParams.instance->name = "initI2C_opt3001";
    taskParams.priority = 1;
    Task_construct(&task0Struct, (Task_FuncPtr)ReadSensorsFxn, &taskParams, NULL);

    Swi_Params_init(&swiParams);
    swiParams.arg0 = 1;
    swiParams.arg1 = 0;
    swiParams.priority = 2;
    swiParams.trigger = 0;
    Swi_construct(&swi0Struct, (Swi_FuncPtr)ADC0_FilterFxn, &swiParams, NULL);
    swi0Handle = Swi_handle(&swi0Struct);

    Swi_Params_init(&swiParams);
    swiParams.arg0 = 1;
    swiParams.arg1 = 0;
    swiParams.priority = 2;
    swiParams.trigger = 0;
    Swi_construct(&swi1Struct, (Swi_FuncPtr)ADC1_FilterFxn, &swiParams, NULL);
    swi1Handle = Swi_handle(&swi1Struct);

    Swi_Params_init(&swiParams);
    swiParams.arg0 = 1;
    swiParams.arg1 = 0;
    swiParams.priority = 2;
    swiParams.trigger = 0;
    Swi_construct(&swi2Struct, (Swi_FuncPtr)ProcessAccelDataFxn, &swiParams, NULL);
    swi2Handle = Swi_handle(&swi2Struct);

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
/*
 * This is the main sensor task that will spin and
 * check sensor data against criteria
 */
void InitI2C_opt3001() {
    lightLimitReached = false;
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

    //Clear interrupt bit
    ReadI2C(i2cHandle, OPT3001_SLAVE_ADDRESS, REG_CONFIGURATION, (uint8_t*)&val);
    IntEnable(INT_GPIOP2);
}

void InitADC0_CurrentSense() {
    ADC0Window.index = 0;
    ADC0Window.sum = 0;
    ADC0Window.avg = 0;

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    //Using timer to trigger sampling
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    //Makes GPIO an INPUT and sets them to be ANALOG
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
    //uint32_t ui32Base, uint32_t ui32SequenceNum, uint32_t ui32Trigger, uint32_t
    ADCSequenceConfigure(ADC0_BASE, ADC_SEQ , ADC_TRIGGER_TIMER, 0);

    //uint32_t ui32Base, uint32_t ui32SequenceNum, uint32_t ui32Step, uint32_t ui32Config
    ADCSequenceStepConfigure( ADC0_BASE, ADC_SEQ , ADC_STEP , ADC_CTL_IE | ADC_CTL_CH0 | ADC_CTL_END);

    //Trigger ADC sampling periodically when timer runs out
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet()/10);
    TimerControlTrigger(TIMER0_BASE, TIMER_A, true);

    //Enable everything
    ADCSequenceEnable(ADC0_BASE, ADC_SEQ);
    ADCIntEnable(ADC0_BASE, ADC_SEQ);
    ADCIntClear( ADC0_BASE, ADC_SEQ);
    IntEnable(INT_ADC0SS1);
    TimerEnable(TIMER0_BASE, TIMER_A);
}

void InitADC1_CurrentSense() {
    ADC1Window.index = 0;
    ADC1Window.sum = 0;
    ADC1Window.avg = 0;

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);

    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_7);

    ADCSequenceConfigure(ADC1_BASE, ADC_SEQ , ADC_TRIGGER_TIMER, 0);

    ADCSequenceStepConfigure(ADC1_BASE, ADC_SEQ , ADC_STEP , ADC_CTL_IE | ADC_CTL_CH4 | ADC_CTL_END);

    ADCSequenceEnable(ADC1_BASE, ADC_SEQ);
    ADCIntEnable(ADC1_BASE, ADC_SEQ);
    ADCIntClear(ADC1_BASE, ADC_SEQ);
    IntEnable(INT_ADC1SS1);
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
    val =  0x0A;
    WriteByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMI160_FOC_CONF_DEFAULT, val);

    //Interrupt raised when data ready
    val =  0x10;
    WriteByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMI160_RA_INT_EN_1, val);

    //Map FIFO full interrupt type to int1
    val =  0x80;
    WriteByteI2C(i2cHandle, BMI160_SLAVE_ADDRESS, BMI160_RA_INT_MAP_1, val);

    //Enable no latching
    val =  0x00;
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

bool SensorOpt3001Read(I2C_Handle i2cHandle, uint16_t *rawData)
{
    bool success;
    uint16_t val;

    success = ReadI2C(i2cHandle, OPT3001_SLAVE_ADDRESS, REG_CONFIGURATION, (uint8_t*)&val);

    if (success) {
        success = ReadI2C(i2cHandle, OPT3001_SLAVE_ADDRESS, REG_RESULT, (uint8_t*)&val);
    }

    if (success) {
        // Swap bytes
        *rawData = (val << 8) | (val>>8 &0xFF);
    }

    return (success);
}

bool BufferReadI2C_OPT3001(I2C_Handle handle, uint8_t slaveAddress, uint8_t ui8Reg, int numBytes)
{
    txBuffer_OPT[0] = ui8Reg;
    i2cTransaction_OPT.slaveAddress = slaveAddress;
    i2cTransaction_OPT.writeBuf = txBuffer_OPT;
    i2cTransaction_OPT.writeCount = 1;
    i2cTransaction_OPT.readBuf = rxBuffer_OPT;
    i2cTransaction_OPT.readCount = numBytes;
    I2C_transfer(handle, &i2cTransaction_OPT);
    return true;
}

bool BufferReadI2C_BMI160(I2C_Handle handle, uint8_t slaveAddress, uint8_t ui8Reg, int numBytes)
{
    txBuffer_BMI[0] = ui8Reg;
    i2cTransaction_BMI.slaveAddress = slaveAddress;
    i2cTransaction_BMI.writeBuf = txBuffer_BMI;
    i2cTransaction_BMI.writeCount = 1;
    i2cTransaction_BMI.readBuf = rxBuffer_BMI;
    i2cTransaction_BMI.readCount = numBytes;
    I2C_transfer(handle, &i2cTransaction_BMI);
    return true;
}

bool ReadI2C(I2C_Handle i2cHandle, uint8_t slaveAddress, uint8_t ui8Reg, uint8_t* data)
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
//    Task_sleep(500);
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


