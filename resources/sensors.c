#include "sensors.h"
#include "bmi160.h"
#include <math.h>

void InitialiseTasks() {
    Task_Params taskParams;
    /* Construct heartBeat Task  thread */
    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task0Stack;
    //taskParams.instance->name = "initI2C_opt3001";
    taskParams.priority = 1;
    Task_construct(&task0Struct, (Task_FuncPtr)ReadSensorsFxn, &taskParams, NULL);

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
    I2C_Params      opt3001Params;
    uint8_t txBuffer[3];
    I2C_Transaction i2cTransaction;

    /* Create I2C for usage */
    I2C_Params_init(&opt3001Params);
    opt3001Params.bitRate = I2C_100kHz;
    opt3001Params.transferMode = I2C_MODE_BLOCKING;
    opt3001 = I2C_open(0, &opt3001Params);

    if (opt3001 == NULL) {
        System_abort("Error Initializing Opt3001 I2C\n");
    } else {
        System_printf("Opt3001 I2C Initialized!\n");
    }

    //Read device ID
    uint16_t data;
    bool success = ReadI2C(opt3001, OPT3001_SLAVE_ADDRESS, REG_DEVICE_ID, &data);
    while(data != 48) {
        System_printf("I2C FAIL\t trying again in 5ms\n");
        System_flush();
        Task_sleep(5);
        bool success = ReadI2C(opt3001, OPT3001_SLAVE_ADDRESS, REG_DEVICE_ID, &data);
    }

    uint16_t val;
    val =  CONFIG_VAL;
    WriteHalfwordI2C(opt3001, OPT3001_SLAVE_ADDRESS, REG_CONFIGURATION, (uint8_t*)&val);

    //Configure the default high/low limits
    SetLowLimit_OPT3001(40.95);
    val =  LOW_LIMIT;
    SetHighLimit_OPT3001(2620.8);


    IntEnable(INT_GPIOP2);
}

void InitADC0_CurrentSense() {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
   // SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOE);

    //Makes GPIO an INPUT and sets them to be ANALOG
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
    //uint32_t ui32Base, uint32_t ui32SequenceNum, uint32_t ui32Trigger, uint32_t
    ADCSequenceConfigure(ADC0_BASE, ADC_SEQ , ADC_TRIGGER_PROCESSOR, 0);

    //uint32_t ui32Base, uint32_t ui32SequenceNum, uint32_t ui32Step, uint32_t ui32Config
    ADCSequenceStepConfigure( ADC0_BASE, ADC_SEQ , ADC_STEP , ADC_CTL_IE | ADC_CTL_CH0 | ADC_CTL_END);

    ADCSequenceEnable(ADC0_BASE, ADC_SEQ);

    ADCIntEnable(ADC0_BASE, ADC_SEQ);

    ADCIntClear( ADC0_BASE, ADC_SEQ);

    IntEnable(INT_ADC0SS1);

    //Start the processing so that callback is called
    ADCProcessorTrigger(ADC0_BASE, ADC_SEQ);
}

void InitADC1_CurrentSense() {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
    //SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

    //Makes GPIO an INPUT and sets them to be ANALOG
    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_7);
    //uint32_t ui32Base, uint32_t ui32SequenceNum, uint32_t ui32Trigger, uint32_t
    ADCSequenceConfigure(ADC1_BASE, ADC_SEQ , ADC_TRIGGER_PROCESSOR, 0);

    //uint32_t ui32Base, uint32_t ui32SequenceNum, uint32_t ui32Step, uint32_t ui32Config
    ADCSequenceStepConfigure(ADC1_BASE, ADC_SEQ , ADC_STEP , ADC_CTL_IE | ADC_CTL_CH4 | ADC_CTL_END);

    ADCSequenceEnable(ADC1_BASE, ADC_SEQ);

    ADCIntEnable(ADC1_BASE, ADC_SEQ);

    ADCIntClear(ADC1_BASE, ADC_SEQ);

    IntEnable(INT_ADC1SS1);

    //Start the processing so that callback is called
    ADCProcessorTrigger(ADC1_BASE, ADC_SEQ);
}

void InitI2C_BMI160() {
    uint8_t val;
    bmi160 = opt3001;

    if (bmi160 == NULL) {
        System_abort("Error Initializing bmi160 I2C\n");
    } else {
        System_printf("bmi160 I2C Initialized!\n");
    }

    uint8_t data;
    uint16_t out;
    ReadByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_RA_CHIP_ID, &data);
    System_printf("Device ID: %d, should be = 209\n", data);
    System_flush();

//    /* Issue a soft-reset to bring the device into a clean state */
    val =  BMI160_CMD_SOFT_RESET;
    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_RA_CMD, val);
    //delay(10);
    Task_sleep(10);

    ReadI2C(bmi160, BMI160_SLAVE_ADDRESS, 0x02, &out);
    System_printf("Error code: %d\n", data);
    System_flush();

    ///* Issue a dummy-read to force the device into SPI comms mode */
    //reg_read(0x7F);
    //delay(1);

    /* Power up the accelerometer */
    val =  BMI160_CMD_ACC_MODE_NORMAL;
    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_RA_CMD, val);
//    reg_write(BMI160_RA_CMD, BMI160_CMD_ACC_MODE_NORMAL);
    //delay(1);
    /* Wait for power-up to complete */
//    while (0x1 != reg_read_bits(BMI160_RA_PMU_STATUS,
//                                BMI160_ACC_PMU_STATUS_BIT,
//                                BMI160_ACC_PMU_STATUS_LEN))
//        delay(1);
    Task_sleep(5);

    ReadI2C(bmi160, BMI160_SLAVE_ADDRESS, 0x03, &out);
    while(out == 0) {
        System_printf("Error code: %d\n", out);
        System_flush();
        ReadI2C(bmi160, BMI160_SLAVE_ADDRESS, 0x03, &out);
        Task_sleep(5);
    }


    /* Power up the gyroscope */
//    val =  BMI160_CMD_GYR_MODE_NORMAL;
//    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_RA_CMD, val);
//    Task_sleep(60);
   // reg_write(BMI160_RA_CMD, BMI160_CMD_GYR_MODE_NORMAL);
    //delay(10);
//    /* Wait for power-up to complete */
//    while (0x1 != reg_read_bits(BMI160_RA_PMU_STATUS,
//                                BMI160_GYR_PMU_STATUS_BIT,
//                                BMI160_GYR_PMU_STATUS_LEN))
//        delay(1);


  //  setFullScaleGyroRange(BMI160_GYRO_RANGE_250);
  //  setFullScaleAccelRange(BMI160_ACCEL_RANGE_2G);

    /* Configure MAG interface and setup mode */
    /* Set MAG interface normal power mode */
//    val =  BMI160_CMD_MAG_MODE_NORMAL;
//    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_RA_CMD, val);
//    Task_sleep(1);
    //reg_write(BMI160_RA_CMD, BMI160_CMD_MAG_MODE_NORMAL);          //Added for BMM150 Support
//    delay(60);


//    /* Sequence for enabling pull-up register */
//    val =  BMI160_FOC_CONF_DEFAULT;
//    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_RA_FOC_CONF, val);
//    //reg_write(BMI160_RA_FOC_CONF, BMI160_FOC_CONF_DEFAULT);        //Added for BMM150 Support
//
//    val =  BMI160_EN_PULL_UP_REG_1;
//    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_RA_CMD, val);
//    //reg_write(BMI160_RA_CMD, BMI160_EN_PULL_UP_REG_1);             //Added for BMM150 Support
//
//    val =  BMI160_EN_PULL_UP_REG_2;
//    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_RA_CMD, val);
//    //reg_write(BMI160_RA_CMD, BMI160_EN_PULL_UP_REG_2);             //Added for BMM150 Support
//
//    val =  BMI160_EN_PULL_UP_REG_3;
//    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_RA_CMD, val);
//    //reg_write(BMI160_RA_CMD, BMI160_EN_PULL_UP_REG_3);             //Added for BMM150 Support
//
//    val =  BMI160_EN_PULL_UP_REG_4;
//    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_7F, val);
//    //reg_write(BMI160_7F, BMI160_EN_PULL_UP_REG_4);                 //Added for BMM150 Support
//
//    val =  BMI160_CMD_GYR_MODE_NORMAL;
//    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_RA_MAG_X_H, val);
//    //reg_write_bits(BMI160_RA_MAG_X_H, 2, 4, 2);                    //Added for BMM150 Support
//
//    val =  BMI160_EN_PULL_UP_REG_5;
//    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_7F, val);
//    //reg_write(BMI160_7F, BMI160_EN_PULL_UP_REG_5);                 //Added for BMM150 Support
//
//
//    /* Set MAG I2C address */
//    val =  BMM150_BASED_I2C_ADDR;
//    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_MAG_IF_0, val);
//    //reg_write(BMI160_MAG_IF_0, BMM150_BASED_I2C_ADDR);             //Added for BMM150 Support


//    /* Enable MAG setup mode, set read out offset to MAX and burst length to 8 */
//    val =  BMI160_MAG_MAN_EN;
//    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_MAG_IF_1, val);
//    //reg_write(BMI160_MAG_IF_1, BMI160_MAG_MAN_EN);                 //Added for BMM150 Support

    /* Enable MAG interface */
    //NEED TO SORT THIS
    //val =  BMI160_EN_PULL_UP_REG_5;
    //WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_IF_CONF, val);
    //reg_write_bits(BMI160_IF_CONF, 2, 4, 2);                       //Added for BMM150 Support

//        /* Configure BMM150 Sensor */
//    /* Enable BMM150 Sleep mode */
//    val =  BMM150_EN_SLEEP_MODE;
//    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_MAG_IF_4, val);
//    //reg_write(BMI160_MAG_IF_4, BMM150_EN_SLEEP_MODE);              //Added for BMM150 Support
//
//    val =  BMM150_POWER_REG;
//    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_MAG_IF_3, val);
//   // reg_write(BMI160_MAG_IF_3, BMM150_POWER_REG);                  //Added for BMM150 Support
//   // delay(3);
//    Task_sleep(3);

//    /* Set BMM150 repetitions for X/Y-Axis */
//    val =  BMM150_REGULAR_REPXY;
//    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_MAG_IF_4, val);
//    //reg_write(BMI160_MAG_IF_4, BMM150_REGULAR_REPXY);             //Added for BMM150 Support
//
//    val =  BMM150_XY_REP_REG;
//    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_MAG_IF_3, val);
//    //reg_write(BMI160_MAG_IF_3, BMM150_XY_REP_REG);                 //Added for BMM150 Support
//
//    /* Set BMM150 repetitions for Z-Axis */
//    val =  BMM150_REGULAR_REPZ;
//    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_MAG_IF_4, val);
//    //reg_write(BMI160_MAG_IF_4, BMM150_REGULAR_REPZ);              //Added for BMM150 Support
//
//    val =  BMM150_Z_REP_REG;
//    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_MAG_IF_3, val);
//   // reg_write(BMI160_MAG_IF_3, BMM150_Z_REP_REG);                  //Added for BMM150 Support
//
//        /* Configure MAG interface for Data mode */
//    /* Configure MAG write address and data to force mode of BMM150 */
//    val =  BMM150_OPMODE_REG_DEFAULT;
//    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_MAG_IF_4, val);
//    //reg_write(BMI160_MAG_IF_4, BMM150_OPMODE_REG_DEFAULT);         //Added for BMM150 Support
//
//    val =  BMM150_OPMODE_REG;
//    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_MAG_IF_3, val);
//   // reg_write(BMI160_MAG_IF_3, BMM150_OPMODE_REG);                 //Added for BMM150 Support
//
//    /* Configure MAG read data address */
//    val =  BMM150_DATA_REG;
//    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_MAG_IF_2, val);
//   // reg_write(BMI160_MAG_IF_2, BMM150_DATA_REG);                //Added for BMM150 Support
//
//    /* Configure MAG interface data rate (25Hz) */
//    val =  BMI160_MAG_CONF_25Hz;
//    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_AUX_ODR_ADDR, val);
//   // reg_write(BMI160_AUX_ODR_ADDR, BMI160_MAG_CONF_25Hz);           //Added for BMM150 Support
//
//    /* Enable MAG data mode */
//    //NEEED TO CHECK THIS
//   // val =  BMI160_EN_PULL_UP_REG_5;
//   // WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_MAG_IF_1, val);
//    //reg_write_bits(BMI160_MAG_IF_1, 0, 7, 1);                      //Added for BMM150 Support
//
////    /* Wait for power-up to complete */
////    while (0x1 != reg_read_bits(BMI160_RA_PMU_STATUS,
////                                BMI160_MAG_PMU_STATUS_BIT,
////                                BMI160_MAG_PMU_STATUS_LEN))
////        delay(1);
//    Task_sleep(1);
//
//
//    /* Only PIN1 interrupts currently supported - map all interrupts to PIN1 */
//    val =  0xFF;
//    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_RA_INT_MAP_0, val);
//    //reg_write(BMI160_RA_INT_MAP_0, 0xFF);
//
//    val =   0xF0;
//    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_RA_INT_MAP_1, val);
//   // reg_write(BMI160_RA_INT_MAP_1, 0xF0);
//
//    val =  0x00;
//    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_RA_INT_MAP_2, val);
//    //reg_write(BMI160_RA_INT_MAP_2, 0x00);

    //Set the bandwidth for accell/gyro to normal and set 100Hz for both
    val =  0x0A;
    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, val, BMI160_FOC_CONF_DEFAULT);

    WriteByteI2C(bmi160, BMI160_SLAVE_ADDRESS, val, BMI160_USER_GYRO_CONFIG_BW__REG);

    ReadByteI2C(bmi160, BMI160_SLAVE_ADDRESS, 0x02, &data);
    System_printf("Error code: %d\n", data);
    System_flush();

    ReadByteI2C(bmi160, BMI160_SLAVE_ADDRESS, 0x03, &data);
    System_printf("Error code: %d\n", data);
    System_flush();

}

bool SensorOpt3001Read(I2C_Handle opt3001, uint16_t *rawData)
{
    bool success;
    uint16_t val;

    success = ReadI2C(opt3001, OPT3001_SLAVE_ADDRESS, REG_CONFIGURATION, &val);

    if (success)
    {
        success = ReadI2C(opt3001, OPT3001_SLAVE_ADDRESS, REG_RESULT, &val);
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

bool SensorBMI160Read(uint16_t *rawData)
{
    bool success;
    uint8_t buffer[2];

    success = BufferReadI2C(bmi160, BMI160_SLAVE_ADDRESS, BMI160_RA_ACCEL_X_L, buffer);

    if (success)
    {
        // Swap bytes
        *rawData = (((uint16_t)buffer[1]) << 8) | buffer[0];
    }
    else
    {
        //    sensorSetErrorData
    }

    System_printf("Accel value: %d\n", *rawData);
    System_flush();

    return (success);
}

bool BufferReadI2C(I2C_Handle i2cHandle, uint8_t slaveAddress, uint8_t ui8Reg, uint8_t data[])
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
    }
    data[0] = rxBuffer[0];
    data[1] = rxBuffer[1];
    return true;
}

bool ReadI2C(I2C_Handle i2cHandle, uint8_t slaveAddress, uint8_t ui8Reg, uint16_t* data)
{

    I2C_Transaction i2cTransaction;
    uint8_t txBuffer[1];
    uint8_t rxBuffer[2];

    txBuffer[0] = ui8Reg;

    i2cTransaction.slaveAddress = slaveAddress;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 3;

    if (!I2C_transfer(i2cHandle, &i2cTransaction)) {
        System_printf("Bad I2C Read transfer!");
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
    WriteHalfwordI2C(opt3001, OPT3001_SLAVE_ADDRESS, REG_HIGH_LIMIT, (uint8_t*)&reg);
}

void SetHighLimit_OPT3001(float val)
{
    uint16_t reg = CalculateLimitReg(val);
    WriteHalfwordI2C(opt3001, OPT3001_SLAVE_ADDRESS, REG_LOW_LIMIT, (uint8_t*)&val);
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


