#include "uart.h"
#include "ringbuf.h"

#include <stdio.h>
#include <stdarg.h>

/* TX ring buffer between app and the (simulated) TXE interrupt. */
static ringbuf_t tx;

void uart_init(void)
{
    ringbuf_init(&tx);
}

void uart_putc(char c)
{
    /* If the buffer is full we flush synchronously rather than drop bytes.
     * On real hardware you might block or drop depending on requirements. */
    if (!ringbuf_put(&tx, (uint8_t)c)) {
        uart_flush();
        ringbuf_put(&tx, (uint8_t)c);
    }
}

void uart_puts(const char *s)
{
    while (*s)
        uart_putc(*s++);
}

void uart_printf(const char *fmt, ...)
{
    char line[256];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(line, sizeof line, fmt, ap);
    va_end(ap);
    uart_puts(line);
}

/* Simulated TXE ISR: shift every queued byte out to the wire. */
void uart_flush(void)
{
    uint8_t b;
    while (ringbuf_get(&tx, &b))
        putchar((int)b);
    fflush(stdout);
}
