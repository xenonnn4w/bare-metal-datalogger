#ifndef LOGSTORE_H
#define LOGSTORE_H

#include <stdint.h>

/* Log storage layer: turns sensor samples into fixed-size records and lays
 * them down on the SPI flash back-to-back, each protected by a CRC. This is
 * the "firmware writing to storage media + verifying it" core of the project.
 *
 * Layout: records start at flash address 0, packed tightly. An erased slot
 * reads back as 0xFFFFFFFF in the seq field, which we treat as end-of-log. */

typedef struct __attribute__((packed)) {
    uint32_t seq;         /* record sequence number (0,1,2,...)            */
    uint32_t tick_ms;     /* fake timestamp                                */
    int32_t  temp_cx100;  /* centi-degrees C                               */
    uint32_t press_pa;    /* pascals                                       */
    uint16_t crc;         /* CRC16-CCITT over the 16 bytes above           */
} record_t;               /* 18 bytes                                      */

#define LOG_ERASED_SEQ  0xFFFFFFFFu

/* Erase the whole chip and reset the write cursor. Call before a fresh run. */
void logstore_format(void);

/* Build a CRC-protected record from a sample and append it to flash.
 * Returns 0 on success, -1 if the flash is full. */
int  logstore_append(uint32_t tick_ms, int32_t temp_cx100, uint32_t press_pa);

/* Read record at index 'i'. Returns:
 *   1  -> valid record placed in *rec, *crc_ok says if CRC matched
 *   0  -> slot is erased (end of log)
 *  -1  -> index out of range */
int  logstore_read(uint32_t i, record_t *rec, int *crc_ok);

#endif
