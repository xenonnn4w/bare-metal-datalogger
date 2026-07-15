#include "sensor.h"

/* --- simulated I2C bus layer -------------------------------------------
 * Stand-ins for the calls a real driver would make. Kept here so the shape
 * of sensor_init()/sensor_read() matches production firmware. */

#define BMP280_ADDR   0x76
#define REG_ID        0xD0
#define BMP280_CHIPID 0x58

static uint8_t i2c_read_reg(uint8_t dev_addr, uint8_t reg)
{
    (void)dev_addr;
    if (reg == REG_ID)
        return BMP280_CHIPID;      /* pretend the device ACKed and replied */
    return 0;
}

/* ----------------------------------------------------------------------- */

int sensor_init(void)
{
    uint8_t id = i2c_read_reg(BMP280_ADDR, REG_ID);
    return (id == BMP280_CHIPID) ? 0 : -1;
}

void sensor_read(uint32_t idx, sensor_sample_t *out)
{
    /* Deterministic sawtooth/wobble so logs are reproducible but not flat.
     * Real driver: read raw ADC regs and apply the calibration formula. */
    int32_t wobble = (int32_t)((idx * 37u) % 500u);        /* 0..499       */
    out->temp_cx100 = 2500 + wobble;                       /* 25.00..29.99C */
    out->press_pa   = 101325u + (idx % 50u) * 3u;          /* ~ sea level  */
}
