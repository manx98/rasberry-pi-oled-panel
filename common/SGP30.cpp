//
// Created by wenyiyu on 2025/4/6.
//
#include "SGP30.h"

#include "log.h"
#include "utils.h"

#include "i2c.h"

/**
 * @file SGP30.c
 *
 * @author Renato Freitas
 * @author Isabella Bologna
 *
 * @brief AS7262 library, based on Adafruit's one.
 * ----> https://www.adafruit.com/products/3779
 * ----> https://github.com/adafruit/Adafruit_AS726x
 *
 * @date 10/2020
 */

/*******************************************
 ********** Private Variables  *************
 *******************************************/

/***********************
 * I²C 16-bit Commands *
 ***********************/
static uint8_t INIT_AIR_QUALITY[2] = {0x20, 0x03};
static uint8_t MEASURE_AIR_QUALITY[2] = {0x20, 0x08};
static uint8_t GET_BASELINE[2] = {0x20, 0x15};
static uint8_t SET_BASELINE[2] = {0x20, 0x1E};
static uint8_t SET_HUMIDITY[2] = {0x20, 0x61};
static uint8_t MEASURE_TEST[2] = {0x20, 0x32};
static uint8_t GET_FEATURE_SET_VERSION[2] = {0x20, 0x2F};
static uint8_t MEASURE_RAW_SIGNALS[2] = {0x20, 0x50};
static uint8_t GET_SERIAL_ID[2] = {0x36, 0x82};
static uint8_t SOFT_RESET[2] = {0x00, 0x06};


static uint8_t SGP_DEVICE_ADDR = 0x58; /**< SGP30 device address variable */


/*******************************************
 ****** Private Functions Prototypes ******
 *******************************************/


/**
 * @brief Executes commands based on SGP30 Command Table
 *
 * @param device        Pointer to sgp30 device
 * @param command       Command to be executed'
 * @param command_len   Command lenght
 * @param delay         Time to wait for a response
 * @param read_data     Buffer where read data will be stored
 * @param read_len      Size of read_data buffer
 *
 * @see https://www.sensirion.com/fileadmin/user_upload/customers/sensirion/Dokumente/9_Gas_Sensors/Datasheets/Sensirion_Gas_Sensors_SGP30_Datasheet.pdf
 *       Table #10
 */
static int sgp30_execute_command(uint8_t command[], uint8_t command_len, uint16_t delay,
                                 uint16_t* read_data, uint8_t read_len);

/**
 * @brief Calculates 8-Bit checksum with given polynomial, used to validate SGP30 commands.
 *
 * @returns 8-bit checksum
 */
static uint8_t sgp30_calculate_CRC(uint8_t* data, uint8_t len);


/*******************************************
 ****** Public Functions Definitions *******
 *******************************************/

int sgp30_init()
{
    uint16_t serial_number[3];
    if (sgp30_execute_command(GET_SERIAL_ID, 2, 10, serial_number, 3) < 0)
    {
        return -1;
    }
    LOG_INFO("Serial Number: {:02x} {:02x} {:02x}", serial_number[0],
                serial_number[1], serial_number[2]);

    uint16_t featureset;
    if (sgp30_execute_command(GET_FEATURE_SET_VERSION, 2, 10, &featureset, 1) < 0)
    {
        return -1;
    }
    LOG_INFO("Feature set version: {:04x}", featureset);
    return sgp30_IAQ_init();
}

void sgp30_softreset()
{
    sgp30_execute_command(SOFT_RESET, 2, 10, NULL, 0);
}

static pthread_mutex_t measure_lock;

int sgp30_IAQ_init()
{
    pthread_mutex_init(&measure_lock, NULL);
    return sgp30_execute_command(INIT_AIR_QUALITY, 2, 10, NULL, 0);
}

int sgp30_IAQ_measure(uint16_t* eco2, uint16_t* tvoc)
{
    pthread_mutex_lock(&measure_lock);
    uint16_t reply[2];
    int ret = 0;
    if (sgp30_execute_command(MEASURE_AIR_QUALITY, 2, 20, reply, 2) < 0)
    {
        ret = -1;
        goto out;
    }
    *tvoc = reply[1];
    *eco2 = reply[0];
out:
    pthread_mutex_unlock(&measure_lock);
    return ret;
}

int sgp30_IAQ_measure_raw(uint16_t* raw_ethanol, uint16_t* raw_H2)
{
    uint16_t reply[2];

    if (sgp30_execute_command(MEASURE_RAW_SIGNALS, 2, 20, reply, 2) < 0)
    {
        return -1;
    }
    *raw_ethanol = reply[1];
    *raw_H2 = reply[0];
    return 0;
}

void sgp30_get_IAQ_baseline(uint16_t* eco2_base, uint16_t* tvoc_base)
{
    uint16_t reply[2];

    sgp30_execute_command(GET_BASELINE, 2, 20, reply, 2);

    *eco2_base = reply[0];
    *tvoc_base = reply[1];
}

void sgp30_set_IAQ_baseline(uint16_t eco2_base, uint16_t tvoc_base)
{
    uint8_t baseline_command[8];

    baseline_command[0] = SET_BASELINE[0];
    baseline_command[1] = SET_BASELINE[1];

    baseline_command[2] = tvoc_base >> 8;
    baseline_command[3] = tvoc_base & 0xFF;
    baseline_command[4] = sgp30_calculate_CRC(baseline_command + 2, 2);

    baseline_command[5] = eco2_base >> 8;
    baseline_command[6] = eco2_base & 0xFF;
    baseline_command[7] = sgp30_calculate_CRC(baseline_command + 5, 2);

    sgp30_execute_command(baseline_command, 8, 20, NULL, 0);
}

void sgp30_set_humidity(uint32_t absolute_humidity)
{
    if (absolute_humidity > 256000)
    {
        LOG_WARN("Abs humidity value {} is too high!", absolute_humidity);
        return;
    }

    uint8_t ah_command[5];
    uint16_t ah_scaled = (uint64_t)absolute_humidity * 256 * 16777 >> 24;

    ah_command[0] = SET_HUMIDITY[0];
    ah_command[1] = SET_HUMIDITY[1];

    ah_command[2] = ah_scaled >> 8;
    ah_command[3] = ah_scaled & 0xFF;
    ah_command[4] = sgp30_calculate_CRC(ah_command + 2, 2);

    sgp30_execute_command(ah_command, 5, 20, NULL, 0);
}


/*******************************************
 ****** Private Functions Definitions ******
 *******************************************/


static int sgp30_execute_command(uint8_t command[], uint8_t command_len, uint16_t delay,
                                 uint16_t* read_data, uint8_t read_len)
{
    /*********************************************************************************************
     ** Measurement routine: START condition, the I2C WRITE header (7-bit I2C device address plus 0
     ** as the write bit) and a 16-bit measurement command.
     **
     ** All commands are listed in TABLE 10 on the datasheet.
     **
     **
     ** After the sensor has completed the measurement, the master can read the measurement results by
     ** sending a START condition followed by an I2C READ header. The sensor will respond with the data.
     *
     *! Each byte must be acknowledged by the microcontroller with an ACK condition for the sensor to continue sending data.
     *! If the sensor does not receive an ACK from the master after any byte of data, it will not continue sending data.
    **********************************************************************************************/

    // Writes SGP30 Command
    int err = i2c_master_write_to_device(SGP_DEVICE_ADDR, command, command_len);

    if (err < 0)
    {
        LOG_WARN("Failed to write SGP30 I2C command!");
        return err;
    }

    // Waits for device to process command and measure desired value
    my_sleep(delay);

    // Checks if there is data to be read from the user, (or if it's just a simple command write)
    if (read_len == 0)
    {
        return 0;
    }

    uint8_t reply_len = read_len * (SGP30_WORD_LEN + 1);
    uint8_t reply_buffer[reply_len];

    // Tries to read device reply
    err = i2c_master_read_to_device(SGP_DEVICE_ADDR, reply_buffer, reply_len);

    if (err < 0)
    {
        LOG_WARN("Failed to read SGP30 I2C command reply! err: 0x{:02x}", err);
        return err; // failed to read reply buffer from chip
    }

    // Calculates expected CRC and compares it with the response
    for (uint8_t i = 0; i < read_len; i++)
    {
        uint8_t crc = sgp30_calculate_CRC(reply_buffer + i * 3, 2);
        LOG_DEBUG("Calc CRC: {:02x},   Reply CRC: {:02x}", crc, reply_buffer[i * 3 + 2]);

        if (crc != reply_buffer[i * 3 + 2])
        {
            LOG_WARN("Reply and Calculated CRCs are different");
            return false;
        }

        // If CRCs are equal, save data
        read_data[i] = reply_buffer[i * 3];
        read_data[i] <<= 8;
        read_data[i] |= reply_buffer[i * 3 + 1];
        LOG_DEBUG("Read data: {:04x}", read_data[i]);
    }
    return 0;
}

static uint8_t sgp30_calculate_CRC(uint8_t* data, uint8_t len)
{
    /**
     ** Data and commands are protected with a CRC checksum to increase the communication reliability.
     ** The 16-bit commands that are sent to the sensor already include a 3-bit CRC checksum.
     ** Data sent from and received by the sensor is always succeeded by an 8-bit CRC.
     *! In write direction it is mandatory to transmit the checksum, since the SGP30 only accepts data if
     *! it is followed by the correct checksum.
     *
     ** In read direction it is up to the master to decide if it wants to read and process the checksum
    */
    uint8_t crc = SGP30_CRC8_INIT;

    for (uint8_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (uint8_t b = 0; b < 8; b++)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ SGP30_CRC8_POLYNOMIAL;
            else
                crc <<= 1;
        }
    }
    return crc;
}
