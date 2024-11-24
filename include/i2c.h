#pragma once

#include <stdint.h>

void i2c_init();
void i2c_write(uint8_t address, const uint8_t *data, uint32_t len);
void i2c_read(uint8_t address, uint8_t reg_address, uint8_t *data, uint32_t len);
