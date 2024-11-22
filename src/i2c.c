#include <system.h>

#define IS_SET(value, mask) ((value) & (mask))

void i2c_init()
{
    I2C1->CTLR1 &= ~I2C_CTLR1_PE;
    I2C1->CTLR2 |= I2C_CTLR2_FREQ & (F_CPU / 1000000);
    I2C1->CKCFGR |= F_CPU / (100000 * 2); // For I2C clock <=100KHz, multiplier is 2
    I2C1->CTLR1 |= I2C_CTLR1_ACK | I2C_CTLR1_PE;

    while (IS_SET(I2C1->STAR2, I2C_STAR2_BUSY))
        ;
}

void i2c_send(uint8_t address, const uint8_t *data, uint32_t len)
{
    I2C1->CTLR1 |= I2C_CTLR1_START;
    while (!IS_SET(I2C1->STAR1, I2C_STAR1_SB))
        ;

    I2C1->DATAR = address; // TODO rw bit
    while (!IS_SET(I2C1->STAR2, I2C_STAR1_ADDR))
        ;

    uint32_t i = 0;
    do
    {
        while (!IS_SET(I2C1->STAR1, I2C_STAR1_TXE))
            ;
        I2C1->DATAR = data[i];
    } while (++i < len);

    while (!IS_SET(I2C1->STAR1, I2C_STAR1_BTF))
        ;
    I2C1->CTLR1 |= I2C_CTLR1_STOP;
}

void i2c_receive()
{
    // TODO
}
