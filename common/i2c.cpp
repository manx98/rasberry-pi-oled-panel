//
// Created by wenyiyu on 2025/4/4.
//
#include "i2c.h"

#include <cerrno>
#include <cstring>
#include <unistd.h>
#include "wiringPi/wiringPiI2C.h"
#include "log.h"

int i2c_master_write_to_device(uint8_t device_address, const uint8_t* write_buffer, size_t write_size)
{
    // 打开 I2C 设备
    int fd = wiringPiI2CSetup(device_address);
    if (fd == -1)
    {
        LOG_ERROR("无法打开 I2C %02x 设备: %s", device_address, strerror(errno));
        return -1;
    }
    // 向 I2C 设备写入寄存器地址
    if (write(fd, write_buffer, write_size) == -1)
    {
        LOG_ERROR("%02x 写入寄存器失败: %s", device_address, strerror(errno));
        goto error;
    }
    close(fd);
    return 0;
error:
    close(fd);
    return -1;
}

int i2c_master_read_to_device(uint8_t device_address, uint8_t* read_buffer, size_t read_size)
{
    // 打开 I2C 设备
    const int fd = wiringPiI2CSetup(device_address);
    if (fd == -1)
    {
        LOG_ERROR("无法打开 I2C %02x 设备: %s", device_address, strerror(errno));
        return -1;
    }
    int ret = -1;
    // 从 I2C 设备读取数据
    if (read(fd, read_buffer, read_size) == -1)
    {
        LOG_ERROR("%02x 读取寄存器失败: %s", device_address, strerror(errno));
        goto error;
    }
    ret = 0;
error:
    close(fd);
    return ret;
}
