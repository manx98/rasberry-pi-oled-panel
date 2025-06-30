/*
 * Driver for Sensirion SHT3x digital temperature and humidity sensor
 * connected to I2C
 *
 * This driver is for the usage with the ESP8266 and FreeRTOS (esp-open-rtos)
 * [https://github.com/SuperHouse/esp-open-rtos]. It is also working with ESP32
 * and ESP-IDF [https://github.com/espressif/esp-idf.git] as well as Linux
 * based systems using a wrapper library for ESP8266 functions.
 *
 * ----------------------------------------------------------------
 *
 * The BSD License (3-clause license)
 *
 * Copyright (c) 2017 Gunar Schorcht (https://github.com/gschorcht)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>
#include <stdlib.h>

#include "i2c.h"
#include "sht3x.h"

#include "log.h"
#include "utils.h"

#define SHT3x_STATUS_CMD               0xF32D
#define SHT3x_CLEAR_STATUS_CMD         0x3041
#define SHT3x_RESET_CMD                0x30A2
#define SHT3x_FETCH_DATA_CMD           0xE000
#define SHT3x_HEATER_OFF_CMD           0x3066

const uint16_t SHT3x_MEASURE_CMD[6][3] = {
    {0x2400, 0x240b, 0x2416}, // [SINGLE_SHOT][H,M,L] without clock stretching
    {0x2032, 0x2024, 0x202f}, // [PERIODIC_05][H,M,L]
    {0x2130, 0x2126, 0x212d}, // [PERIODIC_1 ][H,M,L]
    {0x2236, 0x2220, 0x222b}, // [PERIODIC_2 ][H,M,L]
    {0x2234, 0x2322, 0x2329}, // [PERIODIC_4 ][H,M,L]
    {0x2737, 0x2721, 0x272a}
}; // [PERIODIC_10][H,M,L]

// due to the fact that ticks can be smaller than portTICK_PERIOD_MS, one and
// a half tick period added to the duration to be sure that waiting time for
// the results is long enough
#define TIME_TO_TICKS(ms) (1 + ((ms) + (portTICK_PERIOD_MS-1) + portTICK_PERIOD_MS/2 ) / portTICK_PERIOD_MS)

#define SHT3x_MEAS_DURATION_REP_HIGH   15
#define SHT3x_MEAS_DURATION_REP_MEDIUM 6
#define SHT3x_MEAS_DURATION_REP_LOW    4

// measurement durations in us
const uint16_t SHT3x_MEAS_DURATION_US[3] = {
    SHT3x_MEAS_DURATION_REP_HIGH * 1000,
    SHT3x_MEAS_DURATION_REP_MEDIUM * 1000,
    SHT3x_MEAS_DURATION_REP_LOW * 1000
};

// measurement durations in RTOS ticks
const uint8_t SHT3x_MEAS_DURATION_TICKS[3] = {
    SHT3x_MEAS_DURATION_REP_HIGH,
    SHT3x_MEAS_DURATION_REP_MEDIUM,
    SHT3x_MEAS_DURATION_REP_LOW
};

/** Forward declaration of function for internal use */

static bool sht3x_send_command(uint16_t);
static bool sht3x_read_data(uint8_t*, uint32_t);
static bool sht3x_get_status(uint16_t*);
static bool sht3x_reset();

static uint8_t crc8(const uint8_t data[], int len);
static pthread_mutex_t mutex;
/** ------------------------------------------------ */

int sht3x_init_sensor()
{
    // try to reset the sensor
    if (!sht3x_reset())
    {
        LOG_DEBUG("could not reset the sensor");
    }
    uint16_t status;
    // check again the status after clear status command
    if (!sht3x_get_status(&status))
    {
        LOG_ERROR("could not get sensor status");
        return -1;
    }
    pthread_mutex_init(&mutex, nullptr);
    LOG_DEBUG("sensor initialized");
    return 0;
}


bool sht3x_measure(float* temperature, float* humidity)
{
    pthread_mutex_lock(&mutex);
    bool ret = true;
    sht3x_raw_data_t raw_data;
    if (!sht3x_start_measurement(sht3x_single_shot, sht3x_high))
    {
        ret = false;
        goto out;
    }

    my_sleep(SHT3x_MEAS_DURATION_TICKS[sht3x_high]);


    if (!sht3x_get_raw_data(raw_data))
    {
        ret = false;
        goto out;
    }
    ret = sht3x_compute_values(raw_data, temperature, humidity);
out:
    pthread_mutex_unlock(&mutex);
    return ret;
}


bool sht3x_start_measurement(sht3x_mode_t mode, sht3x_repeat_t repeat)
{
    // start measurement according to selected mode and return an duration estimate
    if (!sht3x_send_command(SHT3x_MEASURE_CMD[mode][repeat]))
    {
        LOG_WARN("could not send start measurment command");
        return false;
    }
    return true;
}

uint8_t sht3x_get_measurement_duration(sht3x_repeat_t repeat)
{
    return SHT3x_MEAS_DURATION_TICKS[repeat]; // in RTOS ticks
}


bool sht3x_get_raw_data(sht3x_raw_data_t raw_data)
{
    // send fetch command in any periodic mode (mode > 0) before read raw data
    if (!sht3x_send_command(SHT3x_FETCH_DATA_CMD))
    {
        LOG_WARN("send fetch command failed");
        return false;
    }

    // read raw data
    if (!sht3x_read_data(raw_data, sizeof(sht3x_raw_data_t)))
    {
        LOG_WARN("read raw data failed");
        return false;
    }

    // check temperature crc
    if (crc8(raw_data, 2) != raw_data[2])
    {
        LOG_WARN("CRC check for temperature data failed");
        return false;
    }

    // check humidity crc
    if (crc8(raw_data + 3, 2) != raw_data[5])
    {
        LOG_WARN("CRC check for humidity data failed");
        return false;
    }
    return true;
}


bool sht3x_compute_values(sht3x_raw_data_t raw_data, float* temperature, float* humidity)
{
    if (!raw_data) return false;

    if (temperature)
        *temperature = ((((raw_data[0] * 256.0) + raw_data[1]) * 175) / 65535.0) - 45;

    if (humidity)
        *humidity = ((((raw_data[3] * 256.0) + raw_data[4]) * 100) / 65535.0);

    return true;
}


bool sht3x_get_results(float* temperature, float* humidity)
{
    sht3x_raw_data_t raw_data;

    if (!sht3x_get_raw_data(raw_data))
        return false;

    return sht3x_compute_values(raw_data, temperature, humidity);
}

static bool sht3x_send_command(uint16_t cmd)
{
    uint8_t data[2] = {static_cast<uint8_t>(cmd >> 8), static_cast<uint8_t>(cmd & 0xff)};

    LOG_DEBUG("send command MSB={:02x} LSB={:02x}", data[0], data[1]);

    int err = i2c_master_write_to_device(SHT3x_ADDR, data, 2);
    if (err < 0)
    {
        LOG_WARN("i2c error on write command {:02x}", cmd);
        return false;
    }

    return true;
}

static bool sht3x_read_data(uint8_t* data, uint32_t len)
{
    int err = i2c_master_read_to_device(SHT3x_ADDR, data, len);
    if (err)
    {
        LOG_WARN("error on read {} byte", len);
        return false;
    }

    return true;
}


static bool sht3x_reset()
{
    LOG_DEBUG("soft-reset triggered");
    // send reset command
    if (!sht3x_send_command(SHT3x_RESET_CMD))
    {
        return false;
    }
    // wait for small amount of time needed (according to datasheet 0.5ms)
    my_sleep(100);

    uint16_t status;

    // check the status after reset
    if (!sht3x_get_status(&status))
        return false;

    return true;
}


static bool sht3x_get_status(uint16_t* status)
{
    uint8_t data[3];

    if (!sht3x_send_command(SHT3x_STATUS_CMD) || !sht3x_read_data(data, 3))
    {
        return false;
    }

    *status = data[0] << 8 | data[1];
    LOG_DEBUG("status={:02x}", *status);
    return true;
}


const uint8_t g_polynom = 0x31;

static uint8_t crc8(const uint8_t data[], int len)
{
    // initialization value
    uint8_t crc = 0xff;

    // iterate over all bytes
    for (int i = 0; i < len; i++)
    {
        crc ^= data[i];

        for (int i1 = 0; i1 < 8; i1++)
        {
            uint8_t xor_value = crc & 0x80;
            crc = crc << 1;
            crc = xor_value ? crc ^ g_polynom : crc;
        }
    }

    return crc;
}

