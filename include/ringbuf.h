#ifndef RINGBUF_H
#define RINGBUF_H

#include <stdint.h>
#include <stddef.h>

/* Classic lock-free-ish single-producer/single-consumer byte ring buffer.
 * On a real MCU this sits between the UART TX ISR and application code:
 * app pushes bytes, the TXE interrupt pops them. Here we drive it manually.
 *
 * Capacity is CAP-1 usable bytes (one slot kept empty to distinguish
 * full from empty without a separate count -> no shared counter to race). */
#define RINGBUF_CAP 256

typedef struct {
    uint8_t  buf[RINGBUF_CAP];
    volatile uint16_t head;   /* producer writes here */
    volatile uint16_t tail;   /* consumer reads here  */
} ringbuf_t;

void   ringbuf_init(ringbuf_t *rb);
int    ringbuf_empty(const ringbuf_t *rb);
int    ringbuf_full(const ringbuf_t *rb);
/* returns 1 on success, 0 if full */
int    ringbuf_put(ringbuf_t *rb, uint8_t byte);
/* returns 1 on success, 0 if empty */
int    ringbuf_get(ringbuf_t *rb, uint8_t *byte);

#endif
