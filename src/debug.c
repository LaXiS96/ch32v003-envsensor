// Extract from https://github.com/cnlohr/ch32v003fun/blob/master/ch32v003fun/ch32v003fun.c

#include <limits.h>
#include <stdarg.h>
#include <system.h>

#include "debug.h"

#define DMDATA0 ((volatile uint32_t*)0xe00000f4)
#define DMDATA1 ((volatile uint32_t*)0xe00000f8)

#define FUNCONF_DEBUGPRINTF_TIMEOUT 0x80000

void handle_debug_input( int numbytes, uint8_t * data ) { (void)numbytes; (void)data; }

static void internal_handle_input( volatile uint32_t * dmdata0 )
{
    uint32_t dmd0 = *dmdata0;
    int bytes = (dmd0 & 0x3f) - 4;
    if( bytes > 0 && bytes < 16 )
    {
        handle_debug_input( bytes, ((uint8_t*)dmdata0) + 1 );
    }
}


void poll_input( void )
{
    volatile uint32_t * dmdata0 = (volatile uint32_t *)DMDATA0;
     if( ((*dmdata0) & 0x80) == 0 )
    {
        internal_handle_input( dmdata0 );
        // Should be 0x80 or so, but for some reason there's a bug that retriggers.
        *dmdata0 = 0x00;
    }
}

#if 0
//           MSB .... LSB
// DMDATA0: char3 char2 char1 [status word]
// where [status word] is:
//   bit 7 = is a "printf" waiting?
//   bit 6 = printf has timed out.
//   bit 0..bit 3 = # of bytes in printf (+4).  (5 or higher indicates a print of some kind)
//     note: if b7 is 0 in reply, but b0..b3 have >=4 then we received data from host.
// Special sentinel:
//     status word = 0x80 = default at start
//     status word = 0xcx = timed out.
// declare as weak to allow overriding.
WEAK int _write(int fd, const char *buf, int size)
{
    (void)fd;
    if( ( *DMDATA0 & 0xc0 ) == 0xc0 ) return 0;

    char buffer[4] = { 0 };
    int place = 0;
    uint32_t lastdmd;
    uint32_t timeout = FUNCONF_DEBUGPRINTF_TIMEOUT; // Give up after ~120ms

    if( size == 0 )
    {
        lastdmd = (*DMDATA0);
        if( lastdmd && !(lastdmd&0x80) ) internal_handle_input( (uint32_t*)DMDATA0 );
    }
    while( place < size )
    {
        int tosend = size - place;
        if( tosend > 7 ) tosend = 7;

        while( ( lastdmd = (*DMDATA0) ) & 0x80 )
        {
            if( timeout-- == 0 )
            {
                *DMDATA0 |= 0xc0;
                return 0;
            }
        }

        if( lastdmd ) internal_handle_input( (uint32_t*)DMDATA0 );

        timeout = FUNCONF_DEBUGPRINTF_TIMEOUT;

        int t = 3;
        while( t < tosend )
        {
            buffer[t-3] = buf[t+place];
            t++;
        }
        *DMDATA1 = *(uint32_t*)&(buffer[0]);
        t = 0;
        while( t < tosend && t < 3 )
        {
            buffer[t+1] = buf[t+place];
            t++;
        }
        buffer[0] = 0x80 | (tosend + 4);
        *DMDATA0 = *(uint32_t*)&(buffer[0]);

        //buf += tosend;
        place += tosend;
    }
    return size;
}
#else
int _write(int fd, const char *buf, int size)
{
    int i = 0;
    int writeSize = size;
    do
    {

        /**
         * data0  data1 8 bytes
         * data0 The lowest byte storage length, the maximum is 7
         *
         */

        while( (*(DMDATA0) != 0u))
        {

        }

        if(writeSize>7)
        {
            *(DMDATA1) = (*(buf+i+3)) | (*(buf+i+4)<<8) | (*(buf+i+5)<<16) | (*(buf+i+6)<<24);
            *(DMDATA0) = (7u) | (*(buf+i)<<8) | (*(buf+i+1)<<16) | (*(buf+i+2)<<24);

            i += 7;
            writeSize -= 7;
        }
        else
        {
            *(DMDATA1) = (*(buf+i+3)) | (*(buf+i+4)<<8) | (*(buf+i+5)<<16) | (*(buf+i+6)<<24);
            *(DMDATA0) = (writeSize) | (*(buf+i)<<8) | (*(buf+i+1)<<16) | (*(buf+i+2)<<24);

            writeSize = 0;
        }

    } while (writeSize);
    return writeSize;
}
#endif

// single to debug intf
WEAK int putchar(int c)
{
    if( ( *DMDATA0 & 0xc0 ) == 0xc0 ) return 0;

    int timeout = FUNCONF_DEBUGPRINTF_TIMEOUT;
    uint32_t lastdmd = 0;

    while( ( lastdmd = (*DMDATA0) ) & 0x80 )
    {
        if( timeout-- == 0 )
        {
            *DMDATA0 |= 0xc0;
            return 0;
        }
    }

    // Simply seeking input.
    if( lastdmd ) internal_handle_input( (uint32_t*)DMDATA0 );

    // Write out character.
    *DMDATA0 = 0x85 | ((const char)c<<8);
    return 1;
}

void SetupDebugPrintf( void )
{
    // Clear out the sending flag.
    *DMDATA1 = 0x00;
    *DMDATA0 = 0x80;
}

void printf(const char *fmt, ...)
{
    char buf[128];
    va_list va;
    va_start(va, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, va);
    va_end(va);
    _write(0, buf, len);
}
