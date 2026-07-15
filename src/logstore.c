#include "logstore.h"
#include "spi_flash.h"
#include "crc16.h"

#include <string.h>

#define REC_SIZE     ((uint32_t)sizeof(record_t))
#define MAX_RECORDS  (FLASH_SIZE / REC_SIZE)
/* CRC covers everything in the record except the 2-byte crc field itself. */
#define REC_CRC_LEN  (REC_SIZE - sizeof(uint16_t))

static uint32_t s_next;   /* write cursor: index of next free record slot */

void logstore_format(void)
{
    flash_erase_chip();
    s_next = 0;
}

int logstore_append(uint32_t tick_ms, int32_t temp_cx100, uint32_t press_pa)
{
    if (s_next >= MAX_RECORDS)
        return -1;                       /* media full */

    record_t rec;
    rec.seq        = s_next;
    rec.tick_ms    = tick_ms;
    rec.temp_cx100 = temp_cx100;
    rec.press_pa   = press_pa;
    rec.crc        = crc16_ccitt(&rec, REC_CRC_LEN);

    uint32_t addr = s_next * REC_SIZE;
    if (flash_write(addr, &rec, REC_SIZE) != 0)
        return -1;

    s_next++;
    return 0;
}

int logstore_read(uint32_t i, record_t *rec, int *crc_ok)
{
    if (i >= MAX_RECORDS)
        return -1;

    uint32_t addr = i * REC_SIZE;
    if (flash_read(addr, rec, REC_SIZE) != 0)
        return -1;

    if (rec->seq == LOG_ERASED_SEQ)
        return 0;                        /* erased slot -> end of log */

    uint16_t want = crc16_ccitt(rec, REC_CRC_LEN);
    *crc_ok = (want == rec->crc);
    return 1;
}
