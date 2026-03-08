# APP 集成说明

## 1. 当前完成范围

这轮已经把原来堆在 `main.c` 里的系统级逻辑拆成了可维护的分层结构，并补齐了当前架构里缺失的关键模块：

- `board/board`
- `drivers/drv_uart`
- `modules/mod_battery_monitor`
- `modules/mod_safety`
- `control/ctrl_speed_loop`
- `app/app`
- `app/app_scheduler`
- `app/app_state_machine`
- `app/app_command`
- `app/app_telemetry`

同时保留并接通了你已经确认可用的主链路：

- `soft_i2c -> mpu6050`
- `imu -> attitude_estimator -> angle_loop`
- VOFA+ 6 通道输出
- `pwm -> tb6612 -> motor`

## 2. 当前总链路

当前主链路已经整理成：

`main.c`
→ `board_build_hw_config()`
→ `app_init()`
→ 驱动 / 设备初始化
→ `app_task()`
→ `mod_imu`
→ `ctrl_attitude_estimator`
→ `ctrl_angle_loop`
→ `ctrl_speed_loop`
→ `mod_motor`
→ `dev_tb6612`
→ `drv_pwm`

其中：

- 电机输出链已经接好
- 但仍由 `motor_output_enable = 0` 默认关闭
- 这样可以先把软件结构、验收链路和答辩口径稳定下来，再单独做电机台架与闭环调试

## 3. 每层当前职责

### `board`

负责板级硬件事实收口：

- 句柄映射
- 引脚映射
- 编码器方向
- PWM 通道分配
- 调试串口选择
- IMU 软件 I2C 引脚与时序默认值
- 默认是否允许电机实际输出

### `drivers`

负责 MCU 外设抽象，当前已经包括：

- `drv_adc`
- `drv_pwm`
- `drv_encoder`
- `drv_soft_i2c`
- `drv_uart`

其中 `drv_uart` 负责：

- 阻塞发送
- 单字节 `RX IT` 启动 / 续收
- `ORE` 清除
- 串口句柄匹配

### `devices`

当前器件层包括：

- `dev_mpu6050`
- `dev_tb6612`

### `modules`

当前功能模块包括：

- `mod_imu`
- `mod_motor`
- `mod_battery_monitor`
- `mod_safety`

### `control`

当前控制层包括：

- `ctrl_attitude_estimator`
- `ctrl_angle_loop`
- `ctrl_speed_loop`

其中速度环当前策略是：

- 先把接口和执行链补齐
- 默认 `enable_closed_loop = 0`
- 等编码器方向和符号确认后再正式闭环

### `app`

当前应用层负责：

- 周期调度
- 状态机
- 命令解析
- 遥测输出
- 安全判定与控制链路装配

## 4. UART 路径现状

当前串口链路已经明确分层：

- `board` 负责选择 `USART1`
- `drv_uart` 负责具体收发与接收恢复
- `app_command` 负责从 FIFO 解析命令
- `app_telemetry` 负责统一文本和 VOFA 输出

注意：

- 主循环不再轮询 `HAL_UART_Receive(..., 1, 0)` 收字节
- 现在是 `USART1 RX` 中断按字节接收
- 接收字节先进入 `app_command` 的环形缓冲
- 主循环只负责非阻塞地从 FIFO 取出一整行并解析

这点对后面答辩很重要：

**现在轮询的是“命令解析过程”，不是“底层串口收字节过程”。**

## 5. `main.c` 现在保留什么

`main.c` 当前只保留三件事：

- CubeMX 生成的时钟和外设初始化
- `board` 资源装配入口
- `app_init()` / `app_task()` 入口

这次重构最关键的结构收益就是：

**让 `main.c` 回到板级入口，不再承载整车业务逻辑。**

## 6. 当前默认策略

为了先把项目写完、而不是过早进入执行调试，当前默认策略是：

- IMU 保持启用
- VOFA 保持启用
- 电池监测启用
- 安全模块启用
- 速度环接口已接入，但默认不开闭环
- 电机执行链已接入，但默认关闭实际输出

## 7. 当前已完成的最小验收

当前已经完成并验证的内容包括：

- `STATUS`
- `ARM`
- `DISARM`
- `CLEAR`
- `TARGET <mdeg>`
- `VOFA ON`
- `VOFA OFF`
- 状态机进入 `READY / ARMED`
- VOFA 观察角度环输出

## 8. 后续自然顺序

后续最自然的顺序是：

1. 收口编码器方向和电机正反语义
2. 单独验证速度环输入符号
3. 再决定是否打开实际电机输出
4. 最后做整车闭环调参
