#include "spi_flash.h"

#include <stdio.h>
#include <string.h>

/* RAM image of the chip. On real hardware every access below would be an
 * SPI transaction (assert /CS, send opcode + address, clock data). */
static uint8_t  s_flash[FLASH_SIZE];
static char     s_path[256];

int flash_init(const char *backing_path)
{
    snprintf(s_path, sizeof s_path, "%s", backing_path);

    FILE *f = fopen(s_path, "rb");
    if (f) {
        size_t n = fread(s_flash, 1, FLASH_SIZE, f);
        fclose(f);
        /* Any tail we didn't read = erased state. */
        if (n < FLASH_SIZE)
            memset(s_flash + n, 0xFF, FLASH_SIZE - n);
    } else {
        /* Fresh chip: erased everywhere. */
        memset(s_flash, 0xFF, FLASH_SIZE);
    }
    return 0;
}

int flash_sync(void)
{
    FILE *f = fopen(s_path, "wb");
    if (!f)
        return -1;
    fwrite(s_flash, 1, FLASH_SIZE, f);
    fclose(f);
    return 0;
}

void flash_erase_chip(void)
{
    memset(s_flash, 0xFF, FLASH_SIZE);
}

void flash_erase_sector(uint32_t addr)
{
    if (addr >= FLASH_SIZE)
        return;
    uint32_t base = addr - (addr % FLASH_SECTOR);
    memset(s_flash + base, 0xFF, FLASH_SECTOR);
}

/* Program one page-bounded chunk. Real NOR: a Page Program command. */
static void page_program(uint32_t addr, const uint8_t *data, size_t len)
{
    for (size_t i = 0; i < len; i++)
        s_flash[addr + i] &= data[i];   /* can only clear bits, never set */
}

int flash_write(uint32_t addr, const void *data, size_t len)
{
    if (addr + len > FLASH_SIZE)
        return -1;

    const uint8_t *p = (const uint8_t *)data;
    while (len) {
        /* Bytes left until the next page boundary. */
        uint32_t page_off = addr % FLASH_PAGE;
        size_t   chunk    = FLASH_PAGE - page_off;
        if (chunk > len)
            chunk = len;

        page_program(addr, p, chunk);
        addr += (uint32_t)chunk;
        p    += chunk;
        len  -= chunk;
    }
    return 0;
}

int flash_read(uint32_t addr, void *out, size_t len)
{
    if (addr + len > FLASH_SIZE)
        return -1;
    memcpy(out, s_flash + addr, len);
    return 0;
}

int flash_inject_fault(uint32_t addr)
{
    if (addr >= FLASH_SIZE)
        return -1;
    s_flash[addr] ^= 0x01;
    return 0;
}
