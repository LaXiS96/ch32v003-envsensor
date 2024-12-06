#define F_CPU 48000000

// Enable to use interrupt-based I2C driver instead of busy-waits
#define USE_I2C_INTERRUPT_DRIVER 1

// Enable to use 64bit arithmetic for higher pressure precision (adds ~1KB of code)
#define BME280_PRESS_COMP_64BIT 0
