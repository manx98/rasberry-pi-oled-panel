//
// Created by bohm on 4/23/24.
//


#include "INA226.h"

#include <pthread.h>
#include <stdint.h>
#include "i2c.h"
#define TAG "INA226"
#define CONFIG_INA226_I2C_ADDR 0x40
static INA226 ina;
#define ToInt16(a) ((int64_t)a[0] << 8 | a[1])

static pthread_mutex_t mutex;

static int INA226_register_read(uint8_t reg_addr, uint8_t* data, size_t len)
{
    if (i2c_master_write_to_device(CONFIG_INA226_I2C_ADDR, &reg_addr, 1) < 0)
    {
        return -1;
    }
    return i2c_master_read_to_device(CONFIG_INA226_I2C_ADDR, data, len);
}

static int INA226_register_write_2_bytes(uint8_t reg_addr, uint16_t value)
{
    const uint8_t write_buf[3] = {reg_addr, static_cast<uint8_t>(value >> 8), static_cast<uint8_t>(value & 0xFF)};

    return i2c_master_write_to_device(CONFIG_INA226_I2C_ADDR, write_buf, sizeof(write_buf));
}

int INA226_configure(ina226_averages_t avg, ina226_busConvTime_t busConvTime,
                     ina226_shuntConvTime_t shuntConvTime, ina226_mode_t mode)
{
    pthread_mutex_init(&mutex, nullptr);
    uint16_t config = 0; // 16679;

    config |= (avg << 9 | busConvTime << 6 | shuntConvTime << 3 | mode);

    ina.vBusMax = 36;
    ina.vShuntMax = 0.08192f;

    int ret = INA226_register_write_2_bytes(INA226_REG_CONFIG, config);
    if (ret != I2C_OK)
    {
        return ret;
    }

    uint8_t data[2];
    ret = INA226_register_read(INA226_REG_CONFIG, data, 2);
    if (ret != I2C_OK)
    {
        return ret;
    }
    int16_t cfg = ToInt16(data);
    return ret;
}

int INA226_calibrate(float rShuntValue, float iMaxExpected)
{
    ina.rShunt = rShuntValue;

    float minimumLSB = iMaxExpected / 32767;

    ina.currentLSB = (uint16_t)(minimumLSB * 100000000);
    ina.currentLSB /= 100000000;
    ina.currentLSB /= 0.0001;
    ina.currentLSB = ceil(ina.currentLSB);
    ina.currentLSB *= 0.0001;

    ina.powerLSB = ina.currentLSB * 25;

    uint16_t calibrationValue = (uint16_t)((0.00512) / (ina.currentLSB * ina.rShunt));

    int ret = INA226_register_write_2_bytes(INA226_REG_CALIBRATION, calibrationValue);
    if (ret != I2C_OK)
    {
        return ret;
    }
    return ret;
}

float INA226_readBusVoltage()
{
    uint8_t data[2];
    pthread_mutex_lock(&mutex);
    int ret = INA226_register_read(INA226_REG_BUSVOLTAGE, data, 2);
    pthread_mutex_unlock(&mutex);
    if (ret != I2C_OK)
    {
        return 0;
    }
    return (float)ToInt16(data) * 0.00125f;
}

float INA226_readBusPower()
{
    uint8_t data[2];
    pthread_mutex_lock(&mutex);
    int ret = INA226_register_read(INA226_REG_POWER, data, 2);
    pthread_mutex_unlock(&mutex);
    if (ret != I2C_OK)
    {
        return 0;
    }
    return ((float)ToInt16(data) * ina.powerLSB);
}

float INA226_readShuntCurrent()
{
    uint8_t data[2];
    int ret = INA226_register_read(INA226_REG_CURRENT, data, 2);
    if (ret != I2C_OK)
    {
        return 0;
    }
    // ESP_LOGI(TAG, "Raw current value read: %d", current);
    // For some reason it needs to be divided by 2...
    return ((float)ToInt16(data) * ina.currentLSB / 2.0f);
}

float INA226_readShuntVoltage()
{
    uint8_t data[2];
    int ret = INA226_register_read(INA226_REG_SHUNTVOLTAGE, data, 2);
    if (ret != I2C_OK)
    {
        return 0;
    }
    return (float)ToInt16(data) * 2.5e-6f; // fixed to 2.5 uV
}
