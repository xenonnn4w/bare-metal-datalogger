#ifndef SENSOR_H
#define SENSOR_H

#include <stdint.h>

/* I2C temperature/pressure sensor driver (simulated, BMP280-style).
 *
 * On real hardware sensor_init() would:
 *   - START, write the 7-bit device address (0x76), write the register
 *     pointer (e.g. 0xD0 "id"), repeated-START, read the id byte, STOP.
 *   - handle ACK/NACK and clock stretching.
 * sensor_read() then reads the raw temp/pressure registers over I2C.
 *
 * Here we synthesise deterministic values from a sample index so runs are
 * reproducible (good for automated verification). */

typedef struct {
    int32_t  temp_cx100;   /* temperature in centi-degrees C (2534 = 25.34C) */
    uint32_t press_pa;     /* pressure in pascals                            */
} sensor_sample_t;

/* Returns 0 on success, -1 if the device id read back is wrong (no device). */
int sensor_init(void);

/* Reads one sample. 'idx' stands in for real elapsed time / register state. */
void sensor_read(uint32_t idx, sensor_sample_t *out);

#endif
