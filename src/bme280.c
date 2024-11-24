// Compensation routines adapted from datasheet: https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf

#include "bme280.h"

#include <system.h>
#include <i2c.h>

#define BME280_I2C_ADDR 0x76
#define BME280_CALIB0 0x88
#define BME280_ID 0xD0
#define BME280_RESET 0xE0
#define BME280_CALIB1 0xE1
#define BME280_CTRL_HUM 0xF2
#define BME280_CTRL_MEAS 0xF4
#define BME280_PRESS 0xF7

static uint16_t dig_T1 = 0;
static int16_t dig_T2 = 0;
static int16_t dig_T3 = 0;
static uint16_t dig_P1 = 0;
static int16_t dig_P2 = 0;
static int16_t dig_P3 = 0;
static int16_t dig_P4 = 0;
static int16_t dig_P5 = 0;
static int16_t dig_P6 = 0;
static int16_t dig_P7 = 0;
static int16_t dig_P8 = 0;
static int16_t dig_P9 = 0;
static uint8_t dig_H1 = 0;
static int16_t dig_H2 = 0;
static uint8_t dig_H3 = 0;
static int16_t dig_H4 = 0;
static int16_t dig_H5 = 0;
static int8_t dig_H6 = 0;

// Returns temperature in DegC, resolution is 0.01 DegC.Output value of “5123” equals 51.23 DegC.
static int32_t compensate_temp(int32_t adc_T, int32_t *t_fine)
{
    int32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) *
            ((int32_t)dig_T3)) >>
           14;
    *t_fine = var1 + var2;
    T = (*t_fine * 5 + 128) >> 8;
    return T;
}

// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
// Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
static uint32_t compensate_press(int32_t adc_P, int32_t t_fine)
{
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;
    if (var1 == 0)
    {
        return 0; // avoid exception caused by division by zero
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    return (uint32_t)p;
}

// Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits).
// Output value of “47445” represents 47445/1024 = 46.333 %RH
static uint32_t compensate_hum(int32_t adc_H, int32_t t_fine)
{
    int32_t v_x1_u32r;
    v_x1_u32r = (t_fine - ((int32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * v_x1_u32r)) +
                   ((int32_t)16384)) >>
                  15) *
                 (((((((v_x1_u32r * ((int32_t)dig_H6)) >> 10) *
                      (((v_x1_u32r * ((int32_t)dig_H3)) >> 11) +
                       ((int32_t)32768))) >>
                     10) +
                    ((int32_t)2097152)) *
                       ((int32_t)dig_H2) +
                   8192) >>
                  14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                               ((int32_t)dig_H1)) >>
                              4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    return (uint32_t)(v_x1_u32r >> 12);
}

void bme280_init()
{
    // Read calibration parameters used by compensation formulas
    uint8_t calib0[25] = {};
    uint8_t calib1[7] = {};
    i2c_read(BME280_I2C_ADDR, BME280_CALIB0, calib0, sizeof(calib0));
    i2c_read(BME280_I2C_ADDR, BME280_CALIB1, calib1, sizeof(calib1));
    dig_T1 = calib0[0] | (calib0[1] << 8);
    dig_T2 = calib0[2] | (calib0[3] << 8);
    dig_T3 = calib0[4] | (calib0[5] << 8);
    dig_P1 = calib0[6] | (calib0[7] << 8);
    dig_P2 = calib0[8] | (calib0[9] << 8);
    dig_P3 = calib0[10] | (calib0[11] << 8);
    dig_P4 = calib0[12] | (calib0[13] << 8);
    dig_P5 = calib0[14] | (calib0[15] << 8);
    dig_P6 = calib0[16] | (calib0[17] << 8);
    dig_P7 = calib0[18] | (calib0[19] << 8);
    dig_P8 = calib0[20] | (calib0[21] << 8);
    dig_P9 = calib0[22] | (calib0[23] << 8);
    dig_H1 = calib0[24];
    dig_H2 = calib1[0] | (calib1[1] << 8);
    dig_H3 = calib1[2];
    dig_H4 = (calib1[3] << 4) | (calib1[4] & 0x0F);
    dig_H5 = (calib1[4] >> 4) | calib1[5];
    dig_H6 = calib1[6];
}

void bme280_measure(int32_t *temp, uint32_t *press, uint32_t *hum)
{
    i2c_write(BME280_I2C_ADDR, (uint8_t[]){BME280_CTRL_HUM, 0b001}, 2);       // Humidity x1 oversampling
    i2c_write(BME280_I2C_ADDR, (uint8_t[]){BME280_CTRL_MEAS, 0b00100101}, 2); // Temperature and pressure x1 oversampling, forced mode

    DLY_ms(20); // TODO measurement duration

    uint8_t data[8] = {};
    i2c_read(BME280_I2C_ADDR, BME280_PRESS, data, sizeof(data));

    int32_t rawpress = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
    int32_t rawtemp = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
    int32_t rawhum = (data[6] << 8) | data[7];

    int32_t t_fine;
    *temp = compensate_temp(rawtemp, &t_fine);
    *press = compensate_press(rawpress, t_fine);
    *hum = compensate_hum(rawhum, t_fine);
}
