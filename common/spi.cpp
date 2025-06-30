//
// Created by wenyiyu on 2025/4/4.
//
#include "spi.h"

#include <stdio.h>
#include <string.h>
#include <wiringPi/wiringPiSPI.h>
#include <errno.h>
#include <stdlib.h>
#include "log.h"

#define SPI_CHANNEL 0
#define SPI_SPEED 80000000
#define SPI_MODE 3

int spi_init()
{
    if (wiringPiSPISetupMode(SPI_CHANNEL, SPI_SPEED, SPI_MODE) < 0)
    {
        LOG_ERROR("SPI init failed： {}", strerror(errno));
        return -1;
    }
    return 0;
}

int spi_write(const uint8_t* data, size_t data_size)
{
#define BUF_BLOCK_SIZE 4096

    uint8_t tmp[BUF_BLOCK_SIZE];
    int ret = 0;
    while (data_size > 0)
    {
        if (data_size < BUF_BLOCK_SIZE)
        {
            memcpy(tmp, data, data_size);
            ret = wiringPiSPIDataRW(0, tmp, data_size);
        } else
        {
            memcpy(tmp, data, BUF_BLOCK_SIZE);
            ret = wiringPiSPIDataRW(0, tmp, BUF_BLOCK_SIZE);
        }
        if (ret < 0)
        {
            LOG_ERROR("SPI write data {} failed: {}", data_size, strerror(errno));
            break;
        }
        if (data_size > BUF_BLOCK_SIZE)
        {
            data += BUF_BLOCK_SIZE;
            data_size -= BUF_BLOCK_SIZE;
        } else
        {
            break;
        }
    }
    return ret;
}
