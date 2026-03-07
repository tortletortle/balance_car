# 变更记录

## 2026-03-07

### [整理] 初始化项目理解文档

- 新增仓库根说明文件，明确当前仅处于整理阶段
- 新增 `balance_v3` 理解文档，整理旧项目结构、硬件映射、控制主线和可复用资产
- 新增工作规则文档，约束“先文档、后提交、再推进”流程
- 新增决策记录文档，固定当前阶段的几条核心原则
- 本次未进入正式项目开发，未新增任何控制固件实现
- 当前 Git 远端尚未配置，因此本轮以本地提交为目标，推送待后续配置

### [整理] 从项目代码提取接线图

- 按用户澄清，放弃图片和残留参考图，不再以 `schematic_converted.md` 作为依据
- 仅基于 `D:\program\learning\balance_v3` 的 `.ioc` 和源码提取真实接线关系
- 新增 `docs/WIRING_FROM_CODE.md`，整理电机驱动、编码器、IMU、ADC、急停、串口和系统引脚
- 明确记录与错误图片版的关键差异，尤其是 `PA4/PA6` 和 `TIM2` 重映射引脚差异
- 本次仍未进入正式项目开发，只完成代码版接线图整理

### [规划] 冻结第 1 步硬件基线

- 新增 `docs/HARDWARE_BASELINE.md`，把当前项目的板级真值正式冻结到原理图 PDF
- 明确区分“板级基线”和“当前旧固件实际使用基线”两层概念
- 固定当前板上的核心模块：电源、主控、TB6612、MPU6050、OLED、蓝牙、超声波、编码器接口、ADC 采样与扩展排针
- 固定当前关键结论：`PA4` 为驱动待机/使能，`PA6` 为 ADC，编码器 B 使用 `PA15/PB3`
- 明确记录当前仍未冻结的项：左右轮物理对应关系、符号方向、扩展模块纳入顺序

### [规划] 完成作业版架构说明

- 新增 `docs/ASSIGNMENT_ARCHITECTURE.md`，正式说明 `board -> drivers -> devices -> modules -> control -> app -> docs` 的分层架构
- 从“职责、边界、依赖、数据流、启动流程、答辩表达”几个维度解释每一层为什么存在
- 明确控制层不直接操作硬件，应用层作为系统调度与状态机总入口

### [规划] 完成目录与文件级蓝图

- 新增 `docs/FILE_LAYOUT_PLAN.md`，把各目录细化到推荐的 `.c/.h` 文件级别
- 为每个目录和文件给出职责说明，作为后续正式开发的落地蓝图
- 在 `docs/DECISIONS.md` 中补充分层架构与控制层边界决策

### [规则] 增加 STM32CubeIDE 工具链约束

- 在 `docs/WORKFLOW.md` 中补充正式开发工具链规则，明确后续统一采用 `STM32CubeIDE` 和 `.ioc`
- 明确外设初始化、时钟、GPIO 复用和基础资源配置以 `CubeMX` 生成为基线
- 明确修改生成文件时优先使用 `USER CODE` 区域，避免破坏再生能力
- 在 `docs/DECISIONS.md` 中补充 `STM32CubeIDE` 工具链和 `CubeMX` 外设规则相关决策

### [规划] 启动 USART1 外设开启方案

- 新增 `docs/USART1_BRINGUP_PLAN.md`，给出 `USART1` 的 Cube 配置、分层位置、分阶段开启策略和验收方式
- 明确 `USART1` 第一阶段以 `115200 8N1` 标准异步串口为基线
- 明确首阶段先做最小发送和基础接收，不直接上 DMA
- 在 `docs/DECISIONS.md` 中补充 `USART1` 分阶段开启决策

### [实现] 完成 USART1 最小发送验证

- 在 `Two_balance_car/Core/Src/main.c` 的 `USER CODE` 区域加入最小串口发送逻辑
- 上电后发送 `BOOT,USART1=OK`
- 主循环每 1 秒发送一次 `HB,USART1=OK`
- 本次只做最小发送验证，不引入 DMA、协议解析或驱动层封装

### [实现] 开通板级全部基础外设

- 更新 `Two_balance_car/Two_balance_car.ioc`，把当前板级外设清单补齐到新工程：`ADC1`、`TIM1`、`TIM2`、`TIM3`、`TIM4`、`USART1`、`USART2` 以及板上相关 GPIO
- 在工程代码中补齐 `adc.c/.h`、`tim.c/.h`、`usart.c/.h`、`gpio.c`、`main.h`、`stm32f1xx_it.c/.h`，使当前工程可直接完成全外设初始化
- `main.c` 初始化序列已扩展为：`GPIO -> ADC1 -> TIM1 -> TIM2 -> TIM3 -> TIM4 -> USART1 -> USART2`
- 本次仅完成“外设开通和工程可编译”，未开始驱动封装、外设启动策略和上层业务逻辑
- 已验证 `STM32CubeIDE` 头less 编译通过

### [修复] 收敛 Main Config 外设警告

- 更新 `Two_balance_car/Two_balance_car.ioc`，清理会触发 `Main Config` 警告的 RCC / USART 参数残留，并修正错误拼接的引脚条目
- 当前 `.ioc` 已通过 `STM32CubeMX` 的 `config load` 快速校验，`NVIC`、`Clock`、`TIM1`、`USART1`、`USART2` 不再报参数错误
- 使用当前工程重新完成代码生成与编译验证，确认本轮收敛没有引入新的工程级错误
- 本次提交只收敛工程配置一致性，不新增控制逻辑、不推进驱动封装

### [实现] 落地首批 drivers 层基础驱动

- 新增 `drivers/drv_encoder.h` 与 `drivers/drv_encoder.c`，把样例中的霍尔编码器读数逻辑重写为基于 HAL 的通用编码器驱动接口
- 新增 `drivers/drv_soft_i2c.h` 与 `drivers/drv_soft_i2c.c`，把样例中的软件 I2C 起始、停止、字节收发与寄存器读写基元提炼为独立驱动层能力
- 本次只实现 `drivers` 层，不把 `MPU6050` 的器件语义混入驱动层；后续 `MPU6050` 将单独进入 `devices`
- 当前驱动代码先按仓库分层落位到 `drivers/`，与 `CubeMX` 生成的板级初始化保持解耦，便于后续逐步接入新工程主线

### [实现] 把首批 drivers 接入 CubeIDE 工程

- 更新 `Two_balance_car/STM32CubeIDE/.project` 与 `Two_balance_car/STM32CubeIDE/.cproject`，把 `drivers` 目录中的首批驱动纳入当前 `STM32CubeIDE` 工程构建与索引路径
- 在 `Two_balance_car/Core/Src/main.c` 中加入最小驱动接入：上电后启动 `TIM2/TIM4` 编码器接口，并初始化 IMU 使用的软件 I2C 总线
- 当前接入只完成“驱动进入工程并可初始化”，还没有开始 `MPU6050` 器件层封装，也没有开始电池电压读取与换算逻辑
### [实现] 补齐 `drv_adc` 与 `dev_mpu6050`

- 新增 `drivers/drv_adc.h` 与 `drivers/drv_adc.c`，把样例中的单次采样、平均采样思路重写为基于 HAL 的 ADC 驱动接口
- 新增 `devices/dev_mpu6050.h`、`devices/dev_mpu6050.c` 与 `devices/dev_mpu6050_reg.h`，把 `MPU6050` 的寄存器定义、初始化流程和原始数据读取放到 `devices` 层
- 更新 `Two_balance_car/Core/Src/main.c`，把 ADC 驱动与 MPU6050 设备做最小初始化接入
- 更新 `STM32CubeIDE` 工程元数据，使 `drivers` 与 `devices` 层新文件进入当前工程构建链
