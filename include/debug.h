#pragma once

#include <mini-printf.h>

#define WEAK __attribute__((weak))

#if 1
static volatile char dbgbuf[64];
static volatile char *pdbgbuf = dbgbuf;
#define dbg_setup()                             \
    {                                           \
        GPIOC->CFGLR |= GPIO_OUT_PP << (4 * 3); \
    }
#define dbg(s)                  \
    {                           \
        GPIOC->OUTDR ^= 1 << 3; \
        *(pdbgbuf++) = (s);     \
    }
#define dbg_print()             \
    {                           \
        *pdbgbuf = '\0';        \
        printf("%s\n", dbgbuf); \
        pdbgbuf = dbgbuf;       \
    }
#else
#define dbg_setup()
#define dbg(...)
#define dbg_print()
#endif

WEAK void handle_debug_input(int numbytes, uint8_t *data);
void poll_input(void);
WEAK int _write(int fd, const char *buf, int size);
WEAK int putchar(int c);
void SetupDebugPrintf(void);
void printf(const char *fmt, ...);
