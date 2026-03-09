# balance_car

这是当前平衡小车重构工程的主项目目录。

## 当前目标

- 按 `board -> drivers -> devices -> modules -> control -> app -> docs` 分层重写工程
- 先把结构、接口和调试链路写清楚，再进入整车闭环调参
- 每完成一块都同步更新文档、提交 Git，并保持改动可解释、可维护

## 当前状态

- `IMU -> attitude_estimator -> angle_loop -> VOFA/STATUS` 链路已打通
- `USART1` 命令接收已改为 `RX 中断 + 环形缓冲`
- `pwm -> tb6612 -> motor` 执行链已接入，但默认不打开实际电机输出
- 速度环接口已补齐，但默认不开闭环
- 正在继续收口 `board` 层、`drv_uart` 层和工程整理

## 目录说明

- `board`：板级资源装配、引脚 / 方向 / 句柄映射
- `drivers`：MCU 外设驱动封装，如 `adc/pwm/encoder/soft_i2c/uart`
- `devices`：具体器件驱动，如 `mpu6050`、`tb6612`
- `modules`：整车功能模块，如 `imu`、`motor`、`battery_monitor`、`safety`
- `control`：姿态估计、角度环、速度环
- `app`：状态机、调度、命令协议、遥测输出
- `docs`：架构说明、调试记录、变更记录
- `Two_balance_car`：STM32CubeIDE 工程主体

## 参考工程

- 第一完成版参考工程：`D:\program\learning\balance_v3`

参考工程仅作为硬件映射、调试思路和控制语义参考，不直接照搬实现。

## 关键文档

- `docs/PROJECT_UNDERSTANDING.md`
- `docs/DEVELOPMENT_MANUAL.md`
- `docs/APP_INTEGRATION.md`
- `docs/DECISIONS.md`
- `docs/CHANGELOG.md`
- `docs/WIRING_FROM_CODE.md`
