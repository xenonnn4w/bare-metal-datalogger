#ifndef SPI_FLASH_H
#define SPI_FLASH_H

#include <stdint.h>
#include <stddef.h>

/* Simulated SPI NOR flash, modelled on a Winbond W25Q-style part.
 *
 * Key physical behaviours that make this "real":
 *  - Erased state is 0xFF. Programming can only turn 1-bits into 0-bits
 *    (implemented as flash[addr] &= data). You MUST erase before you can
 *    write new data over old -- the exact constraint HDD/SSD/NOR firmware
 *    lives with.
 *  - Erase granularity is a SECTOR (4 KB). You can't erase one byte.
 *  - Program granularity is a PAGE (256 B) and a program cannot cross a
 *    page boundary, so flash_write() splits transfers on page lines.
 *
 * Persistence: the whole array is backed by a file (default "flash.bin")
 * so it behaves like non-volatile memory across program runs. */

#define FLASH_SIZE     (64u * 1024u)   /* 64 KB total                     */
#define FLASH_SECTOR   (4u  * 1024u)   /* erase unit                      */
#define FLASH_PAGE     256u            /* program unit                    */

/* Load backing file into RAM (creates a blank, all-0xFF chip if missing). */
int  flash_init(const char *backing_path);
/* Flush RAM image back to the backing file (like closing the NVM). */
int  flash_sync(void);

void flash_erase_chip(void);
void flash_erase_sector(uint32_t addr);   /* addr rounded down to sector  */

/* Program: only works on erased (0xFF) bytes; auto-splits across pages.
 * Returns 0 on success, -1 on out-of-range. */
int  flash_write(uint32_t addr, const void *data, size_t len);

/* Read raw bytes. Returns 0 on success, -1 on out-of-range. */
int  flash_read(uint32_t addr, void *out, size_t len);

/* Fault injection for testing: flip the low bit at 'addr'. Models media
 * bit-rot (a spontaneous cell error), which is NOT a normal program op and
 * so bypasses the program-only-clears-bits rule. Returns 0/-1. */
int  flash_inject_fault(uint32_t addr);

#endif
