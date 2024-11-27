#include <system.h>

#include "i2c.h"

#define I2C_ADDR_WRITE(address) ((address) << 1)
#define I2C_ADDR_READ(address) (((address) << 1) | 1)
// clang-format off
#define I2C_WAIT_BUSY() { while (iss(I2C1->STAR2, I2C_STAR2_BUSY)); }
#define I2C_WAIT_START() { while (!iss(I2C1->STAR1, I2C_STAR1_SB)); }
#define I2C_WAIT_ADDR() { while (!iss(I2C1->STAR1, I2C_STAR1_ADDR)); I2C1->STAR2; } // ADDR is cleared by reading STAR2 after STAR1
#define I2C_WAIT_TXDATA() { while (!iss(I2C1->STAR1, I2C_STAR1_TXE)); }
#define I2C_WAIT_RXDATA() { while (!iss(I2C1->STAR1, I2C_STAR1_RXNE)); }
#define I2C_WAIT_END() { while (!iss(I2C1->STAR1, I2C_STAR1_BTF)); }
// clang-format on

// TODO implement timeouts in busy waits

void i2c_init()
{
    I2C1->CTLR1 &= ~I2C_CTLR1_PE;
    I2C1->CTLR2 |= I2C_CTLR2_FREQ & (F_CPU / 1000000);
    I2C1->CKCFGR |= F_CPU / (100000 * 2); // For I2C clock <=100KHz, multiplier is 2
    I2C1->CTLR1 |= I2C_CTLR1_ACK | I2C_CTLR1_PE;
    I2C_WAIT_BUSY();
}

void i2c_write(uint8_t address, const uint8_t *data, uint32_t len)
{
    I2C_WAIT_BUSY();
    I2C1->CTLR1 |= I2C_CTLR1_START;
    I2C_WAIT_START();

    I2C1->DATAR = I2C_ADDR_WRITE(address);
    I2C_WAIT_ADDR();

    uint32_t i = 0;
    do
    {
        I2C_WAIT_TXDATA();
        I2C1->DATAR = data[i];
    } while (++i < len);

    I2C_WAIT_END();
    I2C1->CTLR1 |= I2C_CTLR1_STOP;
}

void i2c_read(uint8_t address, uint8_t reg_address, uint8_t *data, uint32_t len)
{
    I2C_WAIT_BUSY();
    I2C1->CTLR1 |= I2C_CTLR1_START;
    I2C_WAIT_START();

    I2C1->DATAR = I2C_ADDR_WRITE(address);
    I2C_WAIT_ADDR();

    I2C_WAIT_TXDATA();
    I2C1->DATAR = reg_address;
    I2C_WAIT_END();

    I2C1->CTLR1 |= I2C_CTLR1_START;
    I2C_WAIT_START();

    I2C1->DATAR = I2C_ADDR_READ(address);
    I2C_WAIT_ADDR();

    uint32_t i = 0;
    do
    {
        I2C_WAIT_RXDATA();
        data[i] = (uint8_t)I2C1->DATAR;
    } while (++i < len);

    // It looks like the last byte NACK is handled automatically
    I2C1->CTLR1 |= I2C_CTLR1_STOP;
}
