#ifndef __OPT3001_H__
#define __OPT3001_H__

#include "sensors/sensors.h"

/* OP3001 Register addresses */
//OPT3001 slave address
#define OPT3001_SLAVE_ADDRESS           0x47

#define REG_RESULT                      0x00
#define REG_CONFIGURATION               0x01
#define REG_LOW_LIMIT                   0x02
#define REG_HIGH_LIMIT                  0x03

#define REG_MANUFACTURER_ID             0x7E
#define REG_DEVICE_ID                   0x7F

/* Register values */
#define MANUFACTURER_ID                 0x5449  // TI
#define DEVICE_ID                       0x3001  // Opt 3001
#define CONFIG_RESET                    0xC810
#define CONFIG_TEST                     0xCC10
#define CONFIG_ENABLE                   0x10C4 // 100 ms, continuous
#define CONFIG_DISABLE                  0x10C0 // 0xC010   - 100 ms, shutdown
#define CONFIG_VAL                      0x10C4 // latched window-style comparison and INT pin reports active low
#define CONFIG_HIGHLIGHT_BIT            0x0040 //bit set of config reg if high light event occured
#define CONFIG_LOWLIGHT_BIT             0x0020 //bit set of config reg if low light event occured

#define LOW_LIMIT                       0xFF0F // Full-scale range (Lux) = 40.95
#define HIGH_LIMIT                      0xFF6F // Full scale range (Lux) = 2620.80


/* Bit values */
#define DATA_RDY_BIT                    0x0080  // Data ready

Hwi_Handle hwi_OPT3001;

SlidingWindow_u16 luxValueFilt;
uint16_t rawData;

extern void InitI2C_OPT3001();

extern void OPT3001_ClockHandlerFxn();

extern void ProcessLuxDataFxn();

extern bool GetLuxValue_OPT3001(uint16_t *rawData);

extern void SensorOpt3001Convert(uint16_t rawData, float *convertedLux);

extern void SetLowLimit_OPT3001(float val);

extern void SetHighLimit_OPT3001(float val);

extern bool GetHighLowEventStatus();

extern uint16_t CalculateLimitReg(float luxValue);

#endif // __SENSORS_H__
