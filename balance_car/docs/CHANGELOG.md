# 变更记录

## 2026-03-08

### [Docs] Add development manual

- Add `docs/DEVELOPMENT_MANUAL.md` as a maintainer-facing handbook for reading, extending and debugging the current refactored project
- Summarize layer boundaries, startup flow, runtime control path, command/telemetry path, parameter entry points and safe modification rules
- Add the manual to `README.md` so it can be found quickly from the project root

### [Refactor] Motor profile parameterization

- Add `app_motor_profile_t` to centralize motor-change-sensitive parameters: encoder direction, speed-loop signs, drive-chain signs, PWM limits, deadzone compensation, ramp and mechanical metadata.
- Add `app_logic_config_load_default()` so the default logic config is assembled from one profile instead of a large scattered initializer.
- Sync `motor_profile` into board encoder direction, `ctrl_speed_loop` and `mod_motor` during `app_init()`, so future motor swaps mostly change the profile rather than business logic.
- Add left/right motor command sign fields in `mod_motor`, so motor polarity changes no longer require touching the control chain.

### [收口] 同步 CubeMX 外设工程文件

- 提交 `Two_balance_car.ioc` 与对应生成文件，收口本轮 `NVIC / TIM1 / USART1 / DMA / GPIO` 的外设配置
- 新增 `Core/Inc/dma.h` 与 `Core/Src/dma.c`，使 `USART1` 的 DMA 资源与 `MX_DMA_Init()` 保持工程一致
- 同步 `usart.c`、`stm32f1xx_it.c`、`stm32f1xx_it.h`，补齐 `USART1` 相关 DMA 句柄、IRQ 入口与中断声明
- 同步 `tim.c` 与 `.ioc` 中的中断优先级和 `TIM2` 编码器重映射生成结果，避免 CubeMX 配置与仓库代码继续漂移
- 清理 `main.h` 中已被生成区覆盖的重复宏定义，减少后续再次生成时的混淆
### [重构] 收口 board 与 drv_uart

- 新增 `board/board.c` 与 `board/board.h`，把板级句柄、引脚、方向、默认硬件策略从 `main.c` 抽离到 `board` 层
- `main.c` 只保留 CubeMX 外设初始化、`board` 资源装配入口以及 `app_init/app_task` 入口
- 新增 `drivers/drv_uart.c` 与 `drivers/drv_uart.h`，统一封装串口阻塞发送、单字节中断接收续收和 ORE 清除
- `app_telemetry` 改为通过 `drv_uart` 发送，不再直接调用 `HAL_UART_Transmit`
- `app.c` 改为通过 `drv_uart` 启动 `USART1 RX` 中断接收和错误恢复
- `app_command_poll()` 去掉无效 `UART_HandleTypeDef *` 形参，明确职责为“从 FIFO 解析一行命令”
- 更新 `docs/APP_INTEGRATION.md`，同步当前 UART 实现与 `main.c/board` 分工

### [修正] 串口命令接收改为 RX 中断 + 环形缓冲

- `app_command` 内新增 RX FIFO，解决主循环持续遥测输出时多字节命令丢字节问题
- `USART1` 接收路径改为 `HAL_UART_Receive_IT(..., 1)` + `RxCplt/ErrorCallback` 续收
- 已验证 `STATUS`、`ARM`、`DISARM`、`CLEAR`、`TARGET <mdeg>`、`VOFA ON/OFF` 命令稳定工作
- 自动验收日志保存于 `tmp/uart_acceptance_ring_buffer.txt`

### [集成] app / safety / battery / speed_loop

- 新增 `modules/mod_battery_monitor`，完成 ADC 采样、多次平均、电压换算、滤波与欠压迟滞判断
- 新增 `modules/mod_safety`，统一汇总急停、欠压、IMU 在线 / 超时等安全条件
- 新增 `control/ctrl_speed_loop`，先补齐接口与执行链，默认 `enable_closed_loop = 0`
- 新增 `app/app.c`、`app_scheduler`、`app_state_machine`、`app_command`、`app_telemetry`，把系统级逻辑从 `main.c` 拆到应用层
- 保持 `motor_output_enable = 0` 的保守策略，先完成软件结构和验收，再打开电机输出

### [集成] pwm -> tb6612 -> motor 执行链

- 新增 `drivers/drv_pwm`
- 新增 `devices/dev_tb6612`
- 新增 `modules/mod_motor`
- 完成 `angle_cmd -> mod_motor -> dev_tb6612 -> drv_pwm` 执行链接入
- 默认关闭实际电机输出，避免在方向和符号未确认前直接驱动整车

### [集成] imu -> control -> vofa 观察链

- 新增 `control/ctrl_attitude_estimator`
- 新增 `control/ctrl_angle_loop`
- 打通 `mod_imu -> ctrl_attitude_estimator -> ctrl_angle_loop` 数据流
- 输出 VOFA+ FireWater 6 通道，便于观察目标角、融合角、误差、角度环输出和导数输入

### [整理] IMU bringup 与代码迁移

- 将 IMU bringup 的状态流从 `main.c` 迁移到 `modules/mod_imu`
- 保留 `MPU,WHOAMI`、校零进度、零偏结果等关键串口输出，便于独立审计 IMU 链路
- 恢复 `MPU6050` 的 I2C 总线恢复和 `14-byte burst read` 读数路径

## 2026-03-07

### [整理] 项目理解与规则建立

- 新建仓库内说明文档，明确本项目以“可理解、可维护、可答辩”为首要目标
- 从旧工程代码而不是错误图片中提取接线关系，并形成 `docs/WIRING_FROM_CODE.md`
- 冻结当前板级硬件事实，形成 `docs/HARDWARE_BASELINE.md`
- 明确参考工程 `balance_v3` 只作为参考资产，不直接照搬实现



