# APP 集成说明

## 1. 本轮完成范围

这轮把原来堆在 `main.c` 里的系统级逻辑拆成了可维护的应用层，并补齐了当前架构里缺的核心模块：

- `modules/mod_battery_monitor`
- `modules/mod_safety`
- `control/ctrl_speed_loop`
- `app/app`
- `app/app_scheduler`
- `app/app_state_machine`
- `app/app_command`
- `app/app_telemetry`

同时保留了你已经确认可用的：

- `soft_i2c -> mpu6050`
- `imu -> attitude_estimator -> angle_loop`
- VOFA+ 6 通道输出格式
- `pwm -> tb6612 -> motor` 执行链

## 2. 当前总链路

当前主链路已经整理成：

`main.c`
→ `app_init()`
→ 驱动/设备初始化
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
- 但仍由 `APP_MOTOR_OUTPUT_ENABLE = 0` 默认关闭
- 这样现在可以先把软件结构写完整，再单独做电机台架和闭环调试

## 3. 每层现在负责什么

### `modules/mod_battery_monitor`

负责：

- ADC 多次平均
- 分压换算成电池毫伏值
- 一阶滤波
- 欠压/恢复迟滞判断

输出：

- `raw_value`
- `sense_mv`
- `battery_mv`
- `filtered_battery_mv`
- `undervoltage`

### `modules/mod_safety`

负责统一安全判定，不直接碰控制算法。

当前安全源：

- 急停有效
- 电池欠压
- IMU 不在线
- IMU 数据超时

输出：

- `active_fault_flags`
- `fault_latched_flags`
- `fault_latched`
- `output_allowed`

### `control/ctrl_speed_loop`

当前采用“先把接口补齐、默认不开速度闭环”的思路：

- 输入：左右编码器增量 + `angle_loop` 输出的基础命令
- 内部保留例码思路：
  - 目标速度增量
  - 实际速度增量
  - PI 修正
  - 启动辅助 PWM
  - 限幅
- 默认 `enable_closed_loop = 0`

这样现在链路完整，但不会在你还没做电机方向/编码器符号确认前引入额外变量。

## 4. `app` 层拆分结果

### `app_scheduler`

统一周期调度：

- 心跳
- 电池采样
- 控制步进
- 状态输出

### `app_state_machine`

当前状态：

- `INIT`
- `READY`
- `ARMED`
- `FAULT`
- `ESTOP`

用途是把“能不能输出”从散乱 if 判断里抽出来，后面答辩时也更好讲。

### `app_command`

当前串口命令：

- `HELP`
- `STATUS`
- `ARM`
- `DISARM`
- `CLEAR`
- `TARGET <mdeg>`
- `VOFA ON`
- `VOFA OFF`

全部是轮询非阻塞解析，适合现在这个阶段。

### `app_telemetry`

统一所有串口输出，包括：

- boot
- reset flags
- imu bringup/calibration
- heartbeat
- status
- VOFA+ FireWater

### `app.c`

这是应用层总装配：

- 初始化 drivers/devices/modules/control/app 子模块
- 处理命令
- 跑 IMU 状态机
- 跑安全/状态机
- 到时执行控制步进
- 统一决定是否允许输出到电机

## 5. `main.c` 现在保留什么

`main.c` 现在只保留三件事：

- CubeMX 生成的外设初始化
- 微秒延时函数
- `app_init()` / `app_task()` 入口

这就是这次重构里最重要的结构变化：

**让 `main.c` 回到板级入口，不再承载整车业务逻辑。**

## 6. 当前默认策略

为了先把项目写完、而不是过早进入执行调试，当前默认策略是：

- IMU 保持启用
- VOFA 保持启用
- 电池监测启用
- 安全模块启用
- 速度环接口已接入，但默认不开闭环
- 电机输出链已接入，但默认关闭实际输出

## 7. 这一步完成后你可以继续做什么

后续最自然的顺序是：

1. 烧录验证新 `app` 架构没破坏 IMU/VOFA
2. 验证 `STATUS`/`ARM`/`DISARM`/`TARGET` 命令
3. 核对编码器方向和速度环符号
4. 再决定是否打开 `APP_MOTOR_OUTPUT_ENABLE`
5. 最后做整车闭环调参
