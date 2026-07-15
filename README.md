# Bare-Metal Data Logger (C)

An embedded-firmware project: sample an **I2C sensor**, store each reading as a
**CRC-protected record** on **SPI NOR flash**, and report status over **UART**.
A Python tool reads the log back and produces a **pass/fail verification report**.

The I2C sensor, SPI flash, and UART are simulated in C, so it runs on any PC
with **no hardware**. The drivers are written register-style, so they port to a
real STM32/Blue Pill by swapping the backends.

## Build & run

```sh
make          # build ./datalogger
make demo     # build + run the full end-to-end demo
make clean    # remove binary and flash.bin
```

```sh
./datalogger log 8        # erase flash, take 8 samples, store + print
./datalogger dump         # read records back, verify CRC
./datalogger corrupt 20   # inject a 1-bit media error at flash offset 20
./datalogger dump         # damaged record now shows crc=FAIL
python3 tools/parse_logs.py   # automated pass/fail report (exit 0/1)
```

Flash contents persist in `flash.bin` between commands, like non-volatile memory.

## Design

```
sensor.c        logstore.c            spi_flash.c
(I2C read) ──▶  record + CRC16  ──▶   page-program to NOR flash ──▶ flash.bin
                     │
                     ▼
                  uart.c  (status over ring-buffered TX)
```

- **`spi_flash.c`** — W25Q-style NOR: erased = `0xFF`, program only clears bits
  (erase-before-write), 4 KB sector erase, 256 B page program with boundary
  splitting.
- **`logstore.c`** — packs each sample into an 18-byte `record_t` with a
  CRC16-CCITT; records are contiguous, an erased slot (`seq == 0xFFFFFFFF`)
  marks end-of-log.
- **`uart.c`** — pushes bytes to a ring buffer; `uart_flush()` drains it,
  modelling an interrupt-driven (TXE ISR) TX path.
- **`sensor.c`** — I2C driver (BMP280-style): confirms the device via a chip-ID
  register read, then returns samples.
- **`tools/parse_logs.py`** — runs `dump`, checks every CRC, flags temperature
  anomalies and sequence gaps, exits non-zero on failure.

## Layout

```
include/   driver headers (the API)
src/       crc16, ringbuf, uart, sensor, spi_flash, logstore, main
tools/     parse_logs.py
```

## Skills covered

I2C (addressing, ACK/NACK, chip-ID probe) · SPI NOR (modes, erase-before-write,
sector/page granularity) · UART (ring buffer, ISR-driven TX) · storage firmware
(record format, CRC verification) · automation (Python test harness) ·
architecture (memory-mapped I/O, polling vs interrupt vs DMA).

## Porting to hardware

Replace the simulated backends with real peripheral registers: `sensor.c`'s
`i2c_read_reg` → the I2C peripheral, `spi_flash.c`'s array access → SPI
transactions, `uart.c`'s `putchar` → the USART data register. The upper layers
(`logstore`, CRC, records) stay unchanged.
