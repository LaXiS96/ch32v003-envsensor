// I2c driver implementation that uses interrupts instead of busy waits

#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    int32_t write_len; // Number of bytes to write, if == 0 nothing is written
    uint8_t *write_data;
    int32_t read_len; // Number of bytes to read (after writing), if == 0 nothing is read
    uint8_t *read_data;
    uint8_t address; // Destination device bus address
} I2C_cmd_t;

/// @brief Initialize I2C interrupt-based driver
void i2c_int_init();

/// @brief Start I2C transaction
void i2c_start(I2C_cmd_t cmd);

/// @brief Check if the I2C driver is ready for a new command (previous command has completed)
/// @return true if no transaction is pending, false otherwise
bool i2c_ready();
