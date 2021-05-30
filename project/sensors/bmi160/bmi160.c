#include "bmi160.h"
#include "sensors/sensors.h"

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

    // shift bytes read from bmi160 to form raw accel data
    rawAccel.x = (((int16_t)rxBuffer[1])  << 8) | rxBuffer[0];
    rawAccel.y = (((int16_t)rxBuffer[3])  << 8) | rxBuffer[2];
    rawAccel.z = (((int16_t)rxBuffer[5])  << 8) | rxBuffer[4];

    return true;
}

float CalcAbsoluteAccel() {
    return sqrt(accelXFilt.G * accelXFilt.G) + (accelYFilt.G * accelYFilt.G) + (accelZFilt.G * accelZFilt.G);
}

void ProcessAccelDataFxn() {
    ConvertRawAccelToGs();
    accelXFilt.sum = accelXFilt.sum - accelXFilt.data[accelXFilt.index];
    accelYFilt.sum = accelYFilt.sum - accelYFilt.data[accelYFilt.index];
    accelZFilt.sum = accelZFilt.sum - accelZFilt.data[accelZFilt.index];

    accelXFilt.data[accelXFilt.index] = rawAccel.x;
    accelYFilt.data[accelYFilt.index] = rawAccel.y;
    accelZFilt.data[accelZFilt.index] = rawAccel.z;

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

    //Event_post(sensors_eventHandle, Event_Id_02);
}

void ConvertRawAccelToGs() {
    float scale_2g = 4000.0/BMI160_2G_RANGE;
    float accelX_mG = (float)rawAccel.x * scale_2g;
    float accelY_mG = (float)rawAccel.y * scale_2g;
    float accelZ_mG = (float)rawAccel.z * scale_2g;
    accelXFilt.G = accelX_mG/1000;
    accelYFilt.G = accelY_mG/1000;
    accelZFilt.G = accelZ_mG/1000;
}
