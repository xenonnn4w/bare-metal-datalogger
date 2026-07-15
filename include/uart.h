#ifndef UART_H
#define UART_H

/* UART driver (simulated).
 *
 * Real MCU: writing a byte to the UART data register (e.g. USART->DR) shifts
 * it out one bit at a time; a TXE interrupt fires when the register is free.
 * To avoid blocking the CPU, app code pushes bytes into a TX ring buffer and
 * the ISR drains it. We model exactly that: uart_putc() enqueues, and
 * uart_flush() plays the role of the ISR draining to the "wire" (stdout). */

void uart_init(void);
void uart_putc(char c);
void uart_puts(const char *s);
void uart_printf(const char *fmt, ...);
/* Drain the TX ring buffer to the wire. Stands in for the TXE ISR. */
void uart_flush(void);

#endif
