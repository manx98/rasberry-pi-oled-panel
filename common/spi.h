//
// Created by wenyiyu on 2025/4/4.
//

#ifndef SPI_H
#define SPI_H
#include <stddef.h>
#include <stdint.h>

int spi_init();

int spi_write(const uint8_t* data, size_t data_size);

#endif //SPI_H
