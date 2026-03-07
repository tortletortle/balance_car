# 目录与文件规划

## 1. 文档目的

本文档在正式架构说明的基础上，把项目进一步细化到目录和 `.c/.h` 文件级别，作为后续实现蓝图。

原则：

- 文件职责尽量单一
- 命名统一可解释
- 上层不越级直接操作底层细节

## 2. 推荐目录结构

```text
project_root/
├─ board/
├─ drivers/
├─ devices/
├─ modules/
├─ control/
├─ app/
└─ docs/
```

## 3. `board/` 目录规划

### 3.1 目录职责

- 保存板级硬件定义
- 保存时钟初始化和外设初始化入口
- 保存当前板子的资源分配表

### 3.2 推荐文件

- `board/board.h`
- `board/board.c`
- `board/board_pins.h`
- `board/board_clock.h`
- `board/board_clock.c`
- `board/board_init.h`
- `board/board_init.c`

### 3.3 文件职责

#### `board/board.h`
- 板级公共入口头文件
- 统一导出板级初始化和版本信息

#### `board/board.c`
- 板级公共入口实现
- 统一组织板级初始化调用

#### `board/board_pins.h`
- 定义当前板子的引脚映射
- 如 `DRV_EN`、`ADC`、`PWM_A`、`PWM_B`、`ENC_A`、`ENC_B`

#### `board/board_clock.h`
- 时钟初始化接口声明

#### `board/board_clock.c`
- 系统时钟初始化实现

#### `board/board_init.h`
- 外设初始化接口声明

#### `board/board_init.c`
- GPIO、UART、TIM、ADC、I2C 等初始化组织逻辑

## 4. `drivers/` 目录规划

### 4.1 目录职责

- 提供 MCU 外设驱动封装
- 向上层暴露统一硬件访问接口

### 4.2 推荐文件

- `drivers/drv_uart.h`
- `drivers/drv_uart.c`
- `drivers/drv_pwm.h`
- `drivers/drv_pwm.c`
- `drivers/drv_encoder.h`
- `drivers/drv_encoder.c`
- `drivers/drv_adc.h`
- `drivers/drv_adc.c`
- `drivers/drv_soft_i2c.h`
- `drivers/drv_soft_i2c.c`

### 4.3 文件职责

#### `drv_uart`
- 串口初始化后的基本收发接口
- 可包含阻塞/非阻塞发送、接收缓冲处理

#### `drv_pwm`
- PWM 通道输出设置
- 占空比限制与写入接口

#### `drv_encoder`
- 编码器计数读取
- 增量计算与方向处理基础接口

#### `drv_adc`
- ADC 原始值读取
- 毫伏换算基础接口

#### `drv_soft_i2c`
- 软件 I2C 起始、停止、字节收发、寄存器读写

## 5. `devices/` 目录规划

### 5.1 目录职责

- 面向具体外设芯片进行封装

### 5.2 推荐文件

- `devices/dev_mpu6050.h`
- `devices/dev_mpu6050.c`
- `devices/dev_tb6612.h`
- `devices/dev_tb6612.c`

### 5.3 文件职责

#### `dev_mpu6050`
- 初始化 MPU6050 寄存器
- 读取原始加速度与角速度
- 提供基础量程换算接口

#### `dev_tb6612`
- A/B 通道方向控制
- PWM 与待机脚控制
- 刹车、滑行、停止等基础动作

## 6. `modules/` 目录规划

### 6.1 目录职责

- 对整车级功能进行模块化组织

### 6.2 推荐文件

- `modules/mod_battery_monitor.h`
- `modules/mod_battery_monitor.c`
- `modules/mod_motor.h`
- `modules/mod_motor.c`
- `modules/mod_imu.h`
- `modules/mod_imu.c`
- `modules/mod_safety.h`
- `modules/mod_safety.c`

### 6.3 文件职责

#### `mod_battery_monitor`
- 电池电压滤波
- 电压换算与状态判断
- 对外提供电池状态接口

#### `mod_motor`
- 左右电机目标值管理
- 限幅、斜坡、停止控制
- 将控制层输出转换为执行层目标

#### `mod_imu`
- IMU 数据有效性、新鲜度、零偏管理
- 原始数据缓存和上层查询接口

#### `mod_safety`
- 急停、欠压、超时、故障锁存
- 安全状态判定与允许输出判断

## 7. `control/` 目录规划

### 7.1 目录职责

- 存放控制算法与状态估计逻辑

### 7.2 推荐文件

- `control/ctrl_attitude_estimator.h`
- `control/ctrl_attitude_estimator.c`
- `control/ctrl_angle_loop.h`
- `control/ctrl_angle_loop.c`
- `control/ctrl_speed_loop.h`
- `control/ctrl_speed_loop.c`

### 7.3 文件职责

#### `ctrl_attitude_estimator`
- 对 IMU 数据进行姿态估计
- 输出俯仰角与俯仰角速度

#### `ctrl_angle_loop`
- 角度环控制器实现
- 输入姿态误差，输出平衡控制目标

#### `ctrl_speed_loop`
- 速度环控制器实现
- 输入轮速误差，输出速度补偿量

## 8. `app/` 目录规划

### 8.1 目录职责

- 组织整个系统运行
- 负责状态机、调度和命令协议

### 8.2 推荐文件

- `app/app.h`
- `app/app.c`
- `app/app_state_machine.h`
- `app/app_state_machine.c`
- `app/app_scheduler.h`
- `app/app_scheduler.c`
- `app/app_command.h`
- `app/app_command.c`
- `app/app_telemetry.h`
- `app/app_telemetry.c`

### 8.3 文件职责

#### `app/app`
- 应用层总入口
- 系统初始化完成后的顶层运行接口

#### `app_state_machine`
- 管理系统状态切换
- 如待机、运行、故障、急停等

#### `app_scheduler`
- 管理周期任务调度
- 如 1ms、5ms、20ms、50ms 等任务

#### `app_command`
- 管理串口命令解析
- 管理命令执行入口

#### `app_telemetry`
- 管理状态帧上报
- 统一调试与可观测性接口

## 9. `docs/` 目录规划

### 9.1 目录职责

- 保存项目的设计、流程和调试知识

### 9.2 当前建议文档

- `docs/HARDWARE_BASELINE.md`
- `docs/ASSIGNMENT_ARCHITECTURE.md`
- `docs/FILE_LAYOUT_PLAN.md`
- `docs/PROJECT_UNDERSTANDING.md`
- `docs/WORKFLOW.md`
- `docs/DECISIONS.md`
- `docs/CHANGELOG.md`

## 10. 文件命名规则

推荐统一使用前缀表达层次：

- `board_`：板级
- `drv_`：驱动层
- `dev_`：器件层
- `mod_`：模块层
- `ctrl_`：控制层
- `app_`：应用层

这样做的好处：

- 一眼知道文件属于哪一层
- 便于作业讲解和答辩展示
- 避免命名混乱

## 11. 推荐实现顺序

当前建议后续按以下顺序实现：

1. `board`
2. `drivers`
3. `devices`
4. `modules`
5. `app` 的最小调度与串口
6. `control`
7. `app` 的完整整合

原因：

- 先把底层资源和观测链路打通
- 再上功能模块
- 最后再叠加控制算法

## 12. 当前结论

本文档给出的不是最终代码，而是后续正式开发的文件级蓝图。

后续实现时应满足：

- 每个文件都能说明自己属于哪一层
- 每个文件都能说明为什么不放到别的层
- 每个文件都有明确的输入、输出和责任边界
