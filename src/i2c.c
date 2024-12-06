#include "i2c.h"

#include <system.h>
#include <debug.h>

#if USE_I2C_INTERRUPT_DRIVER

typedef enum
{
    I2C_STATE_IDLE,    // No transaction in progress
    I2C_STATE_CMD,     // Command has been provided, ready to start transaction
    I2C_STATE_START,   // Start condition sent, ready to send write address
    I2C_STATE_WRITE,   // Write address sent, writing bytes
    I2C_STATE_RESTART, // Second start condition sent, ready to send read address
    I2C_STATE_READ,    // Read address sent, reading bytes
} I2C_state;

static volatile I2C_state s_state = I2C_STATE_IDLE;
static volatile I2C_cmd_t s_cmd;

void i2c_init()
{
    NVIC_SetPriority(I2C1_EV_IRQn, 1 << 6);
    NVIC_EnableIRQ(I2C1_EV_IRQn);

    clr(I2C1->CTLR1, I2C_CTLR1_PE);
    sem(I2C1->CTLR2, I2C_CTLR2_FREQ, F_CPU / 1000000);
    set(I2C1->CTLR2, I2C_CTLR2_ITBUFEN | I2C_CTLR2_ITEVTEN);
    sem(I2C1->CKCFGR, I2C_CKCFGR_CCR, F_CPU / (100000 * 2)); // For I2C clock <=100KHz, multiplier is 2
    set(I2C1->CTLR1, I2C_CTLR1_PE);
}

void i2c_start(I2C_cmd_t cmd)
{
    s_cmd = cmd;
    s_state = I2C_STATE_CMD;

    // If only one byte to read preset NACK, else (no reads or read >1byte) ACK
    if (s_cmd.read_len == 1)
        clr(I2C1->CTLR1, I2C_CTLR1_ACK);
    else
        set(I2C1->CTLR1, I2C_CTLR1_ACK);

    // TODO wait until not busy? both the peripheral and the state machine
    dbg('s');
    set(I2C1->CTLR1, I2C_CTLR1_START);
}

bool i2c_ready()
{
    // TODO include bus busy bit or other states?
    return s_state == I2C_STATE_IDLE;
}

void i2c_write(uint8_t address, const uint8_t *data, uint32_t len)
{
    I2C_cmd_t cmd = {
        .address = address,
        .write_data = data,
        .write_len = len,
    };
    i2c_start(cmd);
    while (!i2c_ready())
    {
        dbg('_');
        // TODO should ensure sleep bits are correct
        __WFI();
    }
}

void i2c_read(uint8_t address, uint8_t reg_address, uint8_t *data, uint32_t len)
{
    I2C_cmd_t cmd = {
        .address = address,
        .write_data = (uint8_t[]){reg_address},
        .write_len = 1,
        .read_data = data,
        .read_len = len,
    };
    i2c_start(cmd);
    while (!i2c_ready())
    {
        dbg('_');
        // TODO should ensure sleep bits are correct
        __WFI();
    }
}

__attribute__((interrupt)) void I2C1_EV_IRQHandler()
{
    if (iss(I2C1->STAR1, I2C_STAR1_SB))
    {
        if (s_state == I2C_STATE_CMD)
        {
            s_state = I2C_STATE_START;
            dbg('a');
            I2C1->DATAR = s_cmd.address << 1;
        }
        else if (s_state == I2C_STATE_WRITE)
        {
            s_state = I2C_STATE_RESTART;
            dbg('a');
            I2C1->DATAR = (s_cmd.address << 1) | 1;
        }
    }
    else if (iss(I2C1->STAR1, I2C_STAR1_ADDR))
    {
        if (s_state == I2C_STATE_START)
            s_state = I2C_STATE_WRITE;
        else if (s_state == I2C_STATE_RESTART)
            s_state = I2C_STATE_READ;
        I2C1->STAR2; // Reading STAR2 after STAR1 clears the ADDR bit
    }
    else if (iss(I2C1->STAR1, I2C_STAR1_TXE))
    {
        if (s_state == I2C_STATE_WRITE)
        {
            if (s_cmd.write_len-- > 0) // We still have something to write
            {
                dbg('w');
                I2C1->DATAR = *(s_cmd.write_data++);
            }
            else if (s_cmd.read_len > 0) // We have finished writing and now we have to read
            {
                dbg('s');
                set(I2C1->CTLR1, I2C_CTLR1_START);
                I2C1->DATAR = 0xFF; // Write DATAR to clear TXE (much faster than waiting for BTF)
            }
            else // We have finished writing and had nothing to read
            {
                dbg('p');
                set(I2C1->CTLR1, I2C_CTLR1_STOP);
                s_state = I2C_STATE_IDLE;
                dbg_print();
            }
        }
    }
    else if (iss(I2C1->STAR1, I2C_STAR1_RXNE))
    {
        if (s_state == I2C_STATE_READ)
        {
            if (s_cmd.read_len-- > 0)
            {
                dbg('r');
                if (s_cmd.read_len == 1) // If the next read is the last byte, NACK
                {
                    dbg('n');
                    clr(I2C1->CTLR1, I2C_CTLR1_ACK);
                }
                else if (s_cmd.read_len == 0) // If we read everything, it's time to stop
                {
                    dbg('p');
                    set(I2C1->CTLR1, I2C_CTLR1_STOP);
                    dbg_print();
                }
                *(s_cmd.read_data++) = (uint8_t)I2C1->DATAR;

                if (s_cmd.read_len == 0)
                    s_state = I2C_STATE_IDLE;
            }
        }
    }
    // else
    //     printf("unhandled flag %x\n", I2C1->STAR1);
}

#elif

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
#endif
