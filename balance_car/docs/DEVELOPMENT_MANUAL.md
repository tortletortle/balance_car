# 开发手册

## 1. 手册目的

这份手册面向当前重构版平衡小车工程，目标不是重复讲“平衡车原理”，而是回答下面这些更实际的问题：

- 当前项目到底是怎么分层的
- 代码从哪里开始看最省力
- 上电之后程序按什么顺序运行
- 控制链是怎样从 IMU 一路走到电机输出的
- 后面换电机、加命令、加调试量时应该改哪里
- 哪些地方能改，哪些地方不要乱改

你可以把它当成“进项目后的第一份导航文档”。

## 2. 建议阅读顺序

如果你现在要重新熟悉这个项目，推荐按下面顺序读代码：

1. `Two_balance_car/Core/Src/main.c`
2. `board/board.h` + `board/board.c`
3. `app/app.h` + `app/app.c`
4. `app/app_scheduler.*`
5. `app/app_state_machine.*`
6. `app/app_command.*`
7. `app/app_telemetry.*`
8. `modules/mod_imu.*`
9. `control/ctrl_attitude_estimator.*`
10. `control/ctrl_angle_loop.*`
11. `control/ctrl_speed_loop.*`
12. `modules/mod_motor.*`
13. `devices/dev_tb6612.*`
14. `drivers/*`

这样读的好处是：先看“系统怎么装起来”，再看“数据怎么流动”，最后看“底层细节怎么实现”。

## 3. 仓库分层和边界

当前项目按下面的层次组织：

- `board`
- `drivers`
- `devices`
- `modules`
- `control`
- `app`
- `docs`
- `Two_balance_car`

### 3.1 `Two_balance_car`

这里是 `STM32CubeIDE` / `CubeMX` 工程本体，主要放：

- 时钟初始化
- GPIO / TIM / ADC / USART / DMA 生成代码
- 中断入口
- 链接脚本
- 启动文件

这里的原则是：

- 让 CubeMX 负责“MCU 外设生成”
- 不要把整车业务逻辑重新堆回这里

### 3.2 `board`

`board` 只做一件事：收口当前板子的“硬件事实”。

例如：

- ADC 用哪个句柄
- 左右编码器用哪个定时器
- PWM 用哪个通道
- 调试串口用哪个 UART
- IMU 软件 I2C 用哪两个引脚
- 急停输入口是什么
- 默认是否允许真实电机输出

`board` 不应该写：

- 状态机
- 命令解析
- PID 算法
- VOFA 输出格式

### 3.3 `drivers`

`drivers` 是 MCU 外设抽象层，负责和 HAL 打交道，但不关心整车业务语义。

当前已落地的有：

- `drv_adc`
- `drv_pwm`
- `drv_encoder`
- `drv_soft_i2c`
- `drv_uart`

这里的原则是：

- 只解决“这个外设怎么用”
- 不解决“这个外设为什么这样用”

### 3.4 `devices`

`devices` 面向具体器件：

- `dev_mpu6050`
- `dev_tb6612`

这里负责器件寄存器语义、器件初始化顺序、器件读写细节。

### 3.5 `modules`

`modules` 是功能模块层，把器件能力或驱动能力拼成“整车功能”。

当前包括：

- `mod_imu`
- `mod_motor`
- `mod_battery_monitor`
- `mod_safety`

例如：

- `mod_imu` 不只是读一次 MPU，而是负责启动、校零、运行和报告输出
- `mod_motor` 不只是写 PWM，而是负责限幅、斜坡和停机

### 3.6 `control`

`control` 是控制算法层：

- `ctrl_attitude_estimator`
- `ctrl_angle_loop`
- `ctrl_speed_loop`

这里的原则非常重要：

- 尽量纯算法化
- 不要直接碰 HAL、GPIO、UART
- 输入/输出尽量结构体化

### 3.7 `app`

`app` 是整车装配层，负责把前面各层串起来：

- 初始化顺序
- 周期调度
- 状态机
- 命令协议
- 遥测输出
- 安全判定
- 控制链装配

一句话：

**`app` 是整车大脑的“调度和装配层”，不是底层驱动层。**

## 4. 系统启动流程

### 4.1 启动总流程

```mermaid
flowchart TD
    A[main.c] --> B[HAL_Init]
    B --> C[SystemClock_Config]
    C --> D[MX_GPIO_Init / DMA / ADC / TIM / USART]
    D --> E[board_build_hw_config]
    E --> F[app_logic_config_load_default]
    F --> G[app_init]
    G --> H[驱动初始化]
    H --> I[设备初始化]
    I --> J[模块初始化]
    J --> K[控制器初始化]
    K --> L[调度器 / 状态机 / 命令 / 遥测初始化]
    L --> M[启动串口接收中断]
    M --> N[进入 while(1)]
    N --> O[app_task]
```

### 4.2 关键入口说明

#### `main.c`

`main.c` 现在只保留三类事情：

- CubeMX 生成的时钟和外设初始化
- `board_build_hw_config()`
- `app_logic_config_load_default()` 和 `app_init()` / `app_task()`

这意味着：

- `main.c` 已经不再承载整车业务逻辑
- 以后大部分开发不应该回到 `main.c` 里乱加内容

#### `board_build_hw_config()`

这个函数把硬件资源装进 `board_hw_config_t`，让上层不再直接到处碰全局句柄。

#### `app_logic_config_load_default()`

这个函数负责加载默认软件配置。当前默认配置里最重要的新入口是：

- `motor_profile`

后面换电机时，优先改这里，而不是全工程搜索替换各种神秘数字。

## 5. 主循环和中断路径

### 5.1 主循环

主循环本质上只有一件事：周期调用 `app_task()`。

`app_task()` 内部大体顺序是：

1. 解析串口命令
2. 推进 IMU 状态机
3. 更新调度标志
4. 更新电池监测
5. 更新安全模块和状态机
6. 到控制周期时运行控制链
7. 到状态周期时发送 `STATUS`
8. 到心跳周期时发心跳
9. 如果 VOFA 开着，就发曲线

### 5.2 串口接收路径

当前串口接收不是主循环轮询单字节，而是：

- `USART1 RX` 中断收一个字节
- 回调进入 `app_on_uart_rx_cplt()`
- 字节喂给 `app_command` 的环形缓冲
- 主循环里 `app_command_poll()` 再按行解析命令

所以要记住一句答辩口径：

**现在轮询的是“命令解析”，不是“底层串口收字节”。**

## 6. 控制链如何运行

控制链的软件主路径是：

```text
mod_imu
-> ctrl_attitude_estimator
-> ctrl_angle_loop
-> ctrl_speed_loop
-> mod_motor
-> dev_tb6612
-> drv_pwm
```

### 6.1 `mod_imu`

负责：

- IMU 上电后的等待
- WHOAMI 检查
- 原始数据测试
- 校零采样
- 运行阶段定期产出 `mod_imu_report_t`

### 6.2 `ctrl_attitude_estimator`

把 IMU 原始量变成更稳定的姿态估计量，当前重点输出：

- `pitch_fused_mdeg`
- `pitch_rate_mdps`

它相当于“把传感器数据变成控制器能直接吃的姿态量”。

### 6.3 `ctrl_angle_loop`

输入：

- 目标角 `target_pitch_mdeg`
- 当前角 `pitch_mdeg`
- 当前角速度 `pitch_rate_mdps`

输出：

- `angle_cmd`

这个 `angle_cmd` 不是最终 PWM，而是“为了让车扶正，需要多大控制命令”的中间量。

### 6.4 `ctrl_speed_loop`

输入：

- 左右编码器增量
- 来自角度环的 `base_cmd`

输出：

- `pwm_out`

当前默认策略是：

- 接口已经打通
- 默认 `enable_closed_loop = 0`
- 也就是“链路在，闭环默认先不开”

### 6.5 `mod_motor`

负责：

- PWM 限幅
- 输出斜坡
- 左右电机目标值装配
- 禁止输出时立即归零

### 6.6 安全和状态机为什么不拖慢控制

安全和状态机本质上做的是：

- 判断当前是否允许输出
- 决定系统处于 `INIT / READY / ARMED / FAULT` 哪个状态

它们不是重算法，不会像大滤波器那样拖慢控制。它们更多是：

- 在控制结果真正送去电机之前加一层“门”
- 条件不满足时直接归零

这对整车稳定性反而是加分，而不是减分。

## 7. 当前重要配置入口

### 7.1 电机配置：`motor_profile`

当前最值得记住的配置入口是 `app_motor_profile_t`。

它集中管理后期换电机最容易变化的量：

- 编码器硬件方向
- 速度环使用的编码器符号
- 电机执行链符号
- 速度环闭环开关
- 速度换算比例 `speed_cmd_to_delta_div`
- 速度环 `kp/ki`
- 速度积分限幅
- PWM 上限
- 正反转死区补偿
- 斜坡步长
- 编码器圈数 / 减速比 / 轮半径

**后面换电机时，优先改这一处，不要到处找魔法数。**

### 7.2 角度环参数

角度环配置主要在 `ctrl_angle_loop_config_t`：

- `kp_q8`
- `ki_q8`
- `kd_q8`
- `d_gyro_weight_num/den`
- `d_input_limit_ddeg`
- `d_filter_alpha_num/den`
- `i_accum_limit`
- `cmd_limit`
- `out_offset_cmd`
- `out_offset_enable_cmd`
- `error_deadband_ddeg`

后期真正整车调参时，最先动的一般是：

- `kp_q8`
- `kd_q8`

### 7.3 速度环参数

速度环配置主要在 `ctrl_speed_loop_config_t`：

- `enable_closed_loop`
- `cmd_to_delta_div`
- `encoder_a_sign`
- `encoder_b_sign`
- `kp_q8`
- `ki_q8`
- `i_accum_limit`
- `pwm_limit`
- `startup_pwm_fwd`
- `startup_pwm_rev`
- `startup_assist_min_cmd`

### 7.4 电池与安全参数

电池监测相关：

- `average_sample_count`
- `filter_shift`
- `vref_mv`
- `undervoltage_mv`
- `recover_mv`

安全相关：

- 是否锁存故障 `latch_faults`
- 急停、欠压、IMU 离线、IMU 超时等故障判定

## 8. 串口命令与遥测

### 8.1 当前支持的串口命令

当前代码里已支持：

- `HELP` / `H` / `?`
- `STATUS` / `S`
- `ARM` / `A`
- `DISARM` / `D` / `STOP`
- `CLEAR` / `C`
- `TARGET <mdeg>` / `T <mdeg>`
- `VOFA ON`
- `VOFA OFF`
- `ANGLE` / `PID`
- `ANGLE KP <q8>`
- `ANGLE KI <q8>`
- `ANGLE KD <q8>`
- `ANGLE LIMIT <cmd>`
- `ANGLE DEAD <ddeg>`

其中：

- `ANGLE` / `PID`：打印当前角度环配置
- `ANGLE KP <q8>`：修改角度环比例增益，单位是 `Q8`
- `ANGLE KI <q8>`：修改角度环积分增益，单位是 `Q8`
- `ANGLE KD <q8>`：修改角度环微分增益，单位是 `Q8`
- `ANGLE LIMIT <cmd>`：修改角度环输出限幅
- `ANGLE DEAD <ddeg>`：修改角度误差死区，单位是 `0.1°`

这里的 `Q8` 含义是：实际增益 = 串口写入值 / `256`。

### 8.2 `STATUS` 行怎么看

当前 `STATUS` 的主要字段是：

- `STATE`：状态机状态
- `ARM`：当前 ARM 请求
- `ESTOP`：急停输入状态
- `BAT`：电池电压
- `UV`：欠压状态
- `FAULT`：当前活动故障位
- `LATCH`：已锁存故障位
- `TGT`：目标角
- `PITCH`：融合角
- `RATE`：角速度
- `ERR`：角度误差
- `ACMD`：角度环输出
- `PWM`：速度环 / 电机链当前输出

如果你要看当前角度环参数，不看 `STATUS`，直接发：

- `ANGLE`

当前会返回一行：

- `ANGLE,CFG,PERIOD=...,KPQ8=...,KIQ8=...,KDQ8=...,DLIM=...,DGYRO=.../...,DFILT=.../...,ILIM=...,CLIM=...,DEAD=...`

其中最常用的是：

- `KPQ8` / `KIQ8` / `KDQ8`：当前 PID 参数
- `CLIM`：角度环输出限幅
- `DEAD`：误差死区

### 8.3 启动时串口会打印什么

启动时你会看到几类重要信息：

- `BOOT,USART1=OK`
- 复位来源信息
- IMU bringup / 校零过程
- `MOTOR,CHAIN=OK,...`
- `MOTOR,PROFILE=...`
- `MOTOR,TUNE,...`
- `ANGLE,CFG,...`

其中最后三条就是当前调车相关参数的自报家门。

### 8.4 VOFA 当前 6 通道是什么

当前 `app_telemetry_send_angle_loop_vofa()` 输出 6 个通道，顺序是：

1. 目标角 `target_pitch_mdeg`
2. 融合角 `pitch_fused_mdeg`
3. 角度误差 `pitch_error_mdeg`
4. 角度环输出 `angle_cmd`
5. 融合角速度 `pitch_rate_mdps`
6. 导数滤波输入 `d_input_filt_ddeg`

这 6 个通道适合看：

- 姿态是否正确
- 目标与实际是否同向
- 角度环输出有没有饱和
- 导数量是否太吵

## 9. 后面最常见的开发任务怎么做

### 9.1 换电机

正确做法：

1. 先改 `motor_profile`
2. 确认 `motor_command_sign_a/b`
3. 确认 `encoder_hw_direction_a/b`
4. 确认 `encoder_speed_sign_a/b`
5. 测死区，填 `motor_deadzone_pwm_fwd/rev`
6. 再考虑开速度闭环

不要一上来就：

- 直接改 `mod_motor` 逻辑
- 直接改 `tb6612` 驱动方向语义
- 一边换电机一边调角度环

### 9.2 加一个新串口命令

推荐顺序：

1. 在 `app_command` 里加解析
2. 在 `app_handle_command_result()` 里决定怎么影响系统状态
3. 在 `app_telemetry` 里补对应反馈文本

### 9.3 加一个新的 VOFA 调试量

推荐顺序：

1. 先判断它属于哪一层的输出
2. 尽量不要直接在底层随手 `printf`
3. 如果它是控制链变量，优先从 `app_telemetry_send_angle_loop_vofa()` 统一发

### 9.4 加一个新模块

推荐顺序：

1. 定义它属于 `drivers / devices / modules / control / app` 哪一层
2. 先写 `.h` 的输入输出结构体
3. 再写 `.c` 的初始化、重置、更新函数
4. 最后在 `app_init()` 和 `app_task()` 里接入

## 10. 维护这个架构时的规则

这部分很重要，后面维护时尽量守住：

### 10.1 不要把业务逻辑重新塞回 `main.c`

`main.c` 只保留：

- CubeMX 初始化
- board 装配
- app 入口

### 10.2 `control` 层不要直接碰 HAL

如果控制层直接访问 GPIO/UART/TIM，就会把“算法”和“硬件”重新耦合在一起，后面很难维护。

### 10.3 `drivers` 不要长出整车语义

例如：

- `drv_uart` 不应该知道 `STATUS`
- `drv_pwm` 不应该知道什么叫平衡命令

### 10.4 默认先安全，再调试

当前保守策略是对的：

- 默认电机不实际输出
- 默认速度环不开闭环
- 先看数据，再开执行链

### 10.5 改一块，留痕迹

推荐保持当前习惯：

- 文档同步更新
- `CHANGELOG` 记下本轮改了什么
- 每一阶段形成独立提交

## 11. 当前项目哪些事还没完成

到目前为止，软件主链已经在，但还没完成的主要是：

- 真正打开电机执行输出并做台架验证
- 确认左右轮方向和编码器符号
- 启用速度闭环并整定参数
- 做整车闭环调参
- 根据新电机完善 `motor_profile` 的机械参数

也就是说：

**项目的“结构重写”已经进入后半段，后面重点是执行链和整车联调。**

## 12. 你后面实际最常用的入口

如果你以后很久没看这个项目，重新回来时最优先看这几个地方：

- `Two_balance_car/Core/Src/main.c`
- `board/board.c`
- `app/app.h`
- `app/app.c`
- `app/app_command.c`
- `app/app_telemetry.c`
- `control/ctrl_angle_loop.h`
- `control/ctrl_speed_loop.h`
- `modules/mod_motor.h`

如果你以后换电机，第一眼看：

- `app/app.h` 里的 `app_motor_profile_t`
- `app/app.c` 里的 `g_app_default_motor_profile`

## 13. 一句话总结

这个项目当前最重要的设计思想是：

**让硬件事实、外设访问、器件逻辑、功能模块、控制算法和整车调度各归各位。**

这样你后面换电机、加功能、调参数时，改动范围会小很多，也更容易知道自己到底在改哪一层。
