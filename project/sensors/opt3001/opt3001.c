#include "opt3001.h"
#include "sensors/sensors.h"

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
    SetLowLimit(40.95);
    SetHighLimit(2620.8);

    IntEnable(INT_GPIOP2);
}

bool GetLuxValue(uint16_t *rawData) {
    uint16_t val;
    bool readSuccess = false;

    readSuccess = ReadHalfWordI2C(i2cHandle, OPT3001_SLAVE_ADDRESS, REG_RESULT, (uint8_t*)&val);
    if (readSuccess) *rawData = (val << 8) | (val>>8 & 0xFF);

    return (readSuccess);
}

bool GetHighLowEventStatus() {
    uint16_t val;
    uint16_t rawData;
    bool success;

    success = ReadHalfWordI2C(i2cHandle, OPT3001_SLAVE_ADDRESS, REG_CONFIGURATION, (uint8_t*)&val);
    if (success) {
        // Swap bytes
        rawData = (val << 8) | (val>>8 & 0xFF);
        if(rawData & CONFIG_LOWLIGHT_BIT) {
            return true;
        }
    }

    return false;
}

void ProcessLuxDataFxn() {
    float lux;
    static bool led_state = false;

    ConvertRawDataToLux(rawData, &lux);
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

    //Event_post(sensors_eventHandle, Event_Id_01);
}

void ConvertRawDataToLux(uint16_t rawData, float *convertedLux) {
    uint16_t e, m;

    m = rawData & 0x0FFF;
    e = (rawData & 0xF000) >> 12;

    *convertedLux = m * (0.01 * exp2(e));
}

void SetLowLimit(float val) {
    uint16_t reg = CalculateLimitReg(val);
    WriteHalfwordI2C(i2cHandle, OPT3001_SLAVE_ADDRESS, REG_LOW_LIMIT, (uint8_t*)&reg);
}

void SetHighLimit(float val) {
    uint16_t reg = CalculateLimitReg(val);
    WriteHalfwordI2C(i2cHandle, OPT3001_SLAVE_ADDRESS, REG_HIGH_LIMIT, (uint8_t*)&reg);
}

uint16_t CalculateLimitReg(float luxValue) {
    uint8_t E = 6;
    unsigned char bytes[2];

    //lux equation given in datasheet re-arrages for result bits of reg
    uint16_t result = luxValue / (0.01 * pow(2, E));
    //create bytes of 16bit register
    bytes[0] = (E << 4 & 0x000000F0) | (result >> 8 & 0x0000000F);
    bytes[1] = (result & 0x000000F0) | (result & 0x0000000F);
    //Join bytes to form 16-bit register
    uint16_t reg = bytes[1] << 8 | bytes[0];

    return reg;
}
