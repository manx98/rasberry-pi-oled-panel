//
// Created by wenyiyu on 2025/4/6.
//

#include <string.h>
#include <errno.h>

#include "sensor.h"
#include "wiringPi/wiringPi.h"
#include "spi.h"
#include "INA226.h"
#include "SGP30.h"
#include "sht3x.h"
#include "BMP280.h"
#include "fan.h"
#include "relay.h"
#include "log.h"
#define POWER_PIN 27

static void init_power()
{
    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, HIGH);
}

int init_sensor()
{
    if (wiringPiSetup() < 0) {
        LOG_ERROR("failed to wiringPiSetup: %s", strerror(errno));
        return -1;
    }
    init_power();
#ifdef PANEL_ENABLE_INA226
    int ret = INA226_configure(INA226_AVERAGES_16,
                               INA226_BUS_CONV_TIME_1100US,
                               INA226_SHUNT_CONV_TIME_1100US,
                               INA226_MODE_SHUNT_BUS_CONT);
    if (ret < 0) return ret;
    ret = INA226_calibrate(0.0555f, 3);
    if (ret < 0) return ret;
#endif
#ifdef PANEL_ENABLE_SGP30
    if (sgp30_init() < 0) return -1;
#endif
#ifdef PANEL_ENABLE_SHT3X
    if (sht3x_init_sensor() < 0) return -1;
#endif
#ifdef PANEL_ENABLE_BMP280
    if (bmp280_init() != 0) return -1;
    if (bmp280_set_pressure_oversampling(BMP280_OVERSAMPLING_x4) != 0) return -1;
#endif
#ifdef PANEL_ENABLE_RELAY
    init_relay();
#endif
    return 0;
}
