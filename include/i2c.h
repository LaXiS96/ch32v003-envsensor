#pragma once

#include <stdint.h>
#include <stdbool.h>

/// @brief Initialize I2C driver
void i2c_init();

/// @brief Write (synchronous)
void i2c_write(uint8_t address, const uint8_t *data, uint32_t len);

/// @brief Read from register `reg_address` (synchronous)
void i2c_read(uint8_t address, uint8_t reg_address, uint8_t *data, uint32_t len);

#if USE_I2C_INTERRUPT_DRIVER

typedef struct
{
    int32_t write_len; // Number of bytes to write, if == 0 nothing is written
    uint8_t *write_data;
    int32_t read_len; // Number of bytes to read (after writing), if == 0 nothing is read
    uint8_t *read_data;
    uint8_t address; // Destination device bus address
} I2C_cmd_t;

/// @brief Start I2C transaction (asynchronous)
void i2c_start(I2C_cmd_t cmd);

/// @brief Check if the I2C driver is ready for a new command (previous command has completed)
/// @return true if no transaction is pending, false otherwise
bool i2c_ready();

#endif
