#include "ringbuf.h"

void ringbuf_init(ringbuf_t *rb)
{
    rb->head = 0;
    rb->tail = 0;
}

int ringbuf_empty(const ringbuf_t *rb)
{
    return rb->head == rb->tail;
}

int ringbuf_full(const ringbuf_t *rb)
{
    return (uint16_t)((rb->head + 1) % RINGBUF_CAP) == rb->tail;
}

int ringbuf_put(ringbuf_t *rb, uint8_t byte)
{
    uint16_t next = (uint16_t)((rb->head + 1) % RINGBUF_CAP);
    if (next == rb->tail)
        return 0;                 /* full: drop, caller decides policy */
    rb->buf[rb->head] = byte;
    rb->head = next;
    return 1;
}

int ringbuf_get(ringbuf_t *rb, uint8_t *byte)
{
    if (rb->head == rb->tail)
        return 0;                 /* empty */
    *byte = rb->buf[rb->tail];
    rb->tail = (uint16_t)((rb->tail + 1) % RINGBUF_CAP);
    return 1;
}
