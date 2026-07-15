/* Bare-metal-style data logger (host simulation).
 *
 * Flow mirrors real firmware:
 *   sensor (I2C) --> record + CRC --> SPI NOR flash (NVM)  and  UART (status)
 *
 * Commands:
 *   datalogger log <N>       erase flash, sample N times, store + print
 *   datalogger dump          read every record back, verify CRC, print
 *   datalogger corrupt <off> flip one byte in flash (inject a bit error)
 *
 * The flash is persisted to ./flash.bin so it survives between commands,
 * just like non-volatile storage on a device.
 */
#include "uart.h"
#include "sensor.h"
#include "spi_flash.h"
#include "logstore.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FLASH_FILE "flash.bin"

static int cmd_log(long n)
{
    if (sensor_init() != 0) {
        uart_printf("ERR sensor not responding on I2C\n");
        uart_flush();
        return 1;
    }

    logstore_format();          /* erase chip, reset cursor */
    uart_printf("LOG start n=%ld\n", n);

    for (long i = 0; i < n; i++) {
        sensor_sample_t s;
        sensor_read((uint32_t)i, &s);

        uint32_t tick = (uint32_t)i * 100u;   /* fake 100 ms period */
        if (logstore_append(tick, s.temp_cx100, s.press_pa) != 0) {
            uart_printf("ERR flash full at rec %ld\n", i);
            break;
        }
        uart_printf("  wrote seq=%ld temp=%d.%02d C press=%u Pa\n",
                    i, s.temp_cx100 / 100, abs((int)(s.temp_cx100 % 100)),
                    s.press_pa);
        uart_flush();                          /* drain TX like a TXE ISR */
    }

    flash_sync();               /* commit NVM image to disk */
    uart_printf("LOG done\n");
    uart_flush();
    return 0;
}

static int cmd_dump(void)
{
    uint32_t i = 0;
    int crc_ok;
    record_t rec;
    int rc;

    uart_printf("DUMP start\n");
    while ((rc = logstore_read(i, &rec, &crc_ok)) == 1) {
        uart_printf("REC seq=%u tick=%u temp=%d.%02d press=%u crc=%s\n",
                    rec.seq, rec.tick_ms,
                    rec.temp_cx100 / 100, abs((int)(rec.temp_cx100 % 100)),
                    rec.press_pa, crc_ok ? "OK" : "FAIL");
        uart_flush();
        i++;
    }
    uart_printf("DUMP done count=%u\n", i);
    uart_flush();
    return 0;
}

static int cmd_corrupt(long off)
{
    if (flash_inject_fault((uint32_t)off) != 0) {
        uart_printf("ERR offset out of range\n");
        uart_flush();
        return 1;
    }
    flash_sync();
    uart_printf("FAULT injected at offset %ld (flipped 1 bit)\n", off);
    uart_printf("run 'dump' -> the affected record should show crc=FAIL\n");
    uart_flush();
    return 0;
}

int main(int argc, char **argv)
{
    uart_init();
    flash_init(FLASH_FILE);

    if (argc >= 3 && strcmp(argv[1], "log") == 0)
        return cmd_log(strtol(argv[2], NULL, 10));
    if (argc >= 2 && strcmp(argv[1], "dump") == 0)
        return cmd_dump();
    if (argc >= 3 && strcmp(argv[1], "corrupt") == 0)
        return cmd_corrupt(strtol(argv[2], NULL, 10));

    uart_printf("usage:\n");
    uart_printf("  %s log <N>       sample N times, store to flash\n", argv[0]);
    uart_printf("  %s dump          read back + verify CRC\n", argv[0]);
    uart_printf("  %s corrupt <off> inject a flash error\n", argv[0]);
    uart_flush();
    return 2;
}
