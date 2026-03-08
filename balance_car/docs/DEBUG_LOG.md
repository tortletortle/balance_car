# Debug Log

## 2026-03-08 - Soft I2C startup cleanup

- Stop `MX_GPIO_Init()` from driving `PB10/PB11` low before the software I2C driver takes ownership.
- Change software I2C GPIO configuration to `open-drain + pull-up`.
- Slow software I2C bit delay from `10us` to `20us` for bringup stability.
- Keep the current MPU retry state machine unchanged and continue debugging from the bus layer.

## 2026-03-08 - Soft I2C staged diagnostics

- Add RAM-based soft I2C staged diagnostics for `drv_soft_i2c_read_mem()`.
- Capture last read stage, ACK bit, register address, device address, byte index, and total length.
- Print the diagnostic snapshot from `main.c` when `WHO_AM_I`, raw-test, or runtime MPU reads fail.
- Goal: distinguish whether the failure is at write-address ACK, register ACK, repeated-start, read-address ACK, or data phase.
