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

## 2026-03-08 - Read-address split diagnostics

- Split the repeated-start read-address phase into `RD_ARS` (address byte send) and `RD_ARA` (address ACK).
- Keep other diagnostic stages unchanged so the next halt snapshot can distinguish bit-send vs ACK wait.

## 2026-03-08 - NOP delay A/B test

- Add a build-time switch in `main.c` to choose DWT or NOP-based microsecond delay.
- Set the current test build to `NOP` delay to compare soft I2C stability against the previous DWT version.
- Keep `bit_delay_us` unchanged so the only meaningful variable in this A/B step is the delay implementation.
- Measured result on this board: `48` is stable but too slow (`CAL=OK` about 23.6s), `8` is stable (`CAL=OK` about 6.6s), `6` is also stable and faster (`CAL=OK` about 5.9s).
- Current recommended bringup value is `APP_DELAY_US_NOP_INNER_LOOP = 6U`.
