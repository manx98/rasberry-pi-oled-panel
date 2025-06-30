//
// Created by wenyiyu on 2025/4/4.
//

#ifndef I2C_H
#define I2C_H
#include <cstddef>
#include <cstdint>

#define I2C_OK 0

int i2c_master_write_to_device(uint8_t device_address, const uint8_t* write_buffer, size_t write_size);

int i2c_master_read_to_device(uint8_t device_address, uint8_t* read_buffer, size_t read_size);
#endif //I2C_H
