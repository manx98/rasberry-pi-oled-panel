/**
 * @file SGP30.h
 *
 * @author Renato Freitas
 * @author Isabella Bologna
 *
 * @brief SGP30 library, based on Adafruit's one.
 * ----> https://www.adafruit.com/product/3709
 * ----> https://github.com/adafruit/Adafruit_SGP30
 *
 * @date 10/2020
 */

#ifndef __SGP30_H__
#define __SGP30_H__

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/***********************************
    Default I2C address
************************************/
#define SGP30_ADDR  (0x58)


/****** Commands and constants ******/
#define SGP30_FEATURESET 0x0020    /**< The required set for this library */
#define SGP30_CRC8_POLYNOMIAL 0x31 /**< Seed for SGP30's CRC polynomial */
#define SGP30_CRC8_INIT 0xFF       /**< Init value for CRC */
#define SGP30_WORD_LEN 2           /**< 2 bytes per word */

//!!!
#define NULL_REG 0xFF //?????
//!!!

/**
 *  @brief  Setups the hardware and detects a valid SGP30. Initializes I2C
 *          then reads the serialnumber and checks that we are talking to an SGP30.
 */
int sgp30_init();

/**
 *   @brief  Commands the sensor to perform a soft reset using the "General
 *           Call" mode. Take note that this is not sensor specific and all devices that
 *           support the General Call mode on the on the same I2C bus will perform this.
 */
void sgp30_softreset();

/**
 *   @brief  Commands the sensor to begin the IAQ algorithm. Must be called
 *           after startup.
 */
int sgp30_IAQ_init();

/**
 *  @brief  Commands the sensor to take a single eCO2/VOC measurement. Places
 *          results in sensor.TVOC and sensor.eCO2
 */

int sgp30_IAQ_measure(uint16_t* eco2, uint16_t* tvoc);

/**
 *  @brief  Commands the sensor to take a single H2/ethanol raw measurement.
 *          Places results in sensor.raw_H2 and sensor.raw_ethanol
 */
int sgp30_IAQ_measure_raw(uint16_t* raw_ethanol, uint16_t* raw_H2);

/*!
 *   @brief  Request baseline calibration values for both CO2 and TVOC IAQ
 *           calculations. Places results in parameter memory locaitons.
 *   @param  eco2_base
 *           A pointer to a uint16_t which we will save the calibration
 *           value to
 *   @param  tvoc_base
 *           A pointer to a uint16_t which we will save the calibration value to
 */
void sgp30_get_IAQ_baseline(uint16_t* eco2_base, uint16_t* tvoc_base);

/**
 *  @brief  Assign baseline calibration values for both CO2 and TVOC IAQ
 *          calculations.
 *  @param  eco2_base
 *          A uint16_t which we will save the calibration value from
 *  @param  tvoc_base
 *          A uint16_t which we will save the calibration value from
 */
void sgp30_set_IAQ_baseline(uint16_t eco2_base, uint16_t tvoc_base);

/**
 *  @brief  Set the absolute humidity value [mg/m^3] for compensation to
 *          increase precision of TVOC and eCO2.
 *  @param  absolute_humidity
 *          A uint32_t [mg/m^3] which we will be used for compensation.
 *          If the absolute humidity is set to zero, humidity compensation
 *          will be disabled.
 */
void sgp30_set_humidity(uint32_t absolute_humidity);


#endif  // __SGP30_H__
