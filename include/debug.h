#pragma once

#define WEAK __attribute__((weak))

WEAK void handle_debug_input(int numbytes, uint8_t *data);
void poll_input(void);
WEAK int _write(int fd, const char *buf, int size);
WEAK int putchar(int c);
void SetupDebugPrintf(void);
