#ifndef CRC16_H
#define CRC16_H

#include <stdint.h>
#include <stddef.h>

/* CRC16-CCITT (poly 0x1021, init 0xFFFF).
 * Used to verify record integrity read back from flash = "firmware
 * verification" story: catches bit-rot / bad writes on the media. */
uint16_t crc16_ccitt(const void *data, size_t len);

#endif
