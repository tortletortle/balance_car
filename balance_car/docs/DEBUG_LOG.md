# Debug Log

## 2026-03-08 - Soft I2C startup cleanup

- Stop `MX_GPIO_Init()` from driving `PB10/PB11` low before the software I2C driver takes ownership.
- Change software I2C GPIO configuration to `open-drain + pull-up`.
- Slow software I2C bit delay from `10us` to `20us` for bringup stability.
- Keep the current MPU retry state machine unchanged and continue debugging from the bus layer.
