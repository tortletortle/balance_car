# 代码版接线图（来源：`balance_v3`）

## 1. 说明

本文档只根据项目代码和 `D:\program\learning\balance_v3\balance.ioc` 提取接线关系。

本次提取**不参考**以下内容：

- `D:\program\balance_car\schematic_converted.md`
- 上级目录中的 `_crop_*`、`_new_*`、`_v2_*`、`_z_*` 图片

这些图片和残留文档不作为当前结论依据。

## 2. 提取依据

主要依据文件：

- `D:\program\learning\balance_v3\balance.ioc`
- `D:\program\learning\balance_v3\Core\Inc\main.h`
- `D:\program\learning\balance_v3\Core\Src\gpio.c`
- `D:\program\learning\balance_v3\Core\Src\tim.c`
- `D:\program\learning\balance_v3\Core\Src\adc.c`
- `D:\program\learning\balance_v3\Core\Src\usart.c`
- `D:\program\learning\balance_v3\Core\Src\soft_i2c.c`
- `D:\program\learning\balance_v3\Core\Src\balance_app.c`

## 3. 总体结论

从代码看，这一版项目实际接入的硬件主要是：

- 电机驱动模块
- 两路编码器
- MPU6050 IMU
- 电池电压采样
- 急停输入
- USART1 调试串口

代码中**没有**看到 OLED、蓝牙、超声波等模块的实际启用配置，因此当前接线图不把它们纳入有效接线结论。

## 4. 信号到引脚映射

| 功能 | MCU 引脚 | 代码依据 | 说明 |
|---|---|---|---|
| `DRV_EN` | `PA4` | `main.h` / `balance.ioc` / `gpio.c` | 电机驱动使能 |
| `ESTOP` | `PA5` | `main.h` / `balance.ioc` / `gpio.c` | 急停输入，上拉输入 |
| `BAT_ADC` | `PA6` | `adc.c` / `balance.ioc` | 电池电压 ADC1_IN6 |
| `USART1_TX` | `PA9` | `usart.c` / `balance.ioc` | 调试串口发送 |
| `USART1_RX` | `PA10` | `usart.c` / `balance.ioc` | 调试串口接收 |
| `PWM_B` | `PB0` | `tim.c` / `balance_app.c` | `TIM3_CH3`，电机 B PWM |
| `PWM_A` | `PB1` | `tim.c` / `balance_app.c` | `TIM3_CH4`，电机 A PWM |
| `BIN1` | `PB12` | `main.h` / `gpio.c` / `balance.ioc` | 电机 B 方向脚 1 |
| `BIN2` | `PB13` | `main.h` / `gpio.c` / `balance.ioc` | 电机 B 方向脚 2 |
| `AIN1` | `PB14` | `main.h` / `gpio.c` / `balance.ioc` | 电机 A 方向脚 1 |
| `AIN2` | `PB15` | `main.h` / `gpio.c` / `balance.ioc` | 电机 A 方向脚 2 |
| `ENC_B_CH1` | `PA15` | `tim.c` / `balance.ioc` | `TIM2_CH1`，编码器 B 通道 1 |
| `ENC_B_CH2` | `PB3` | `tim.c` / `balance.ioc` | `TIM2_CH2`，编码器 B 通道 2 |
| `ENC_A_CH1` | `PB6` | `tim.c` / `balance.ioc` | `TIM4_CH1`，编码器 A 通道 1 |
| `ENC_A_CH2` | `PB7` | `tim.c` / `balance.ioc` | `TIM4_CH2`，编码器 A 通道 2 |
| `IMU_SCL` | `PB10` | `soft_i2c.c` | 软件 I2C SCL |
| `IMU_SDA` | `PB11` | `soft_i2c.c` | 软件 I2C SDA |
| `SWDIO` | `PA13` | `balance.ioc` | SWD 调试 |
| `SWCLK` | `PA14` | `balance.ioc` | SWD 调试 |
| `HSE_IN` | `PD0` | `balance.ioc` | 外部高速时钟 |
| `HSE_OUT` | `PD1` | `balance.ioc` | 外部高速时钟 |

## 5. 模块接线图

### 5.1 电机驱动模块

项目代码把电机分成 A/B 两路，而不是左/右。

#### 电机 A

- `PB1` -> `PWM_A` -> 驱动器 PWM 输入 A
- `PB14` -> `AIN1` -> 驱动器方向输入 A1
- `PB15` -> `AIN2` -> 驱动器方向输入 A2

#### 电机 B

- `PB0` -> `PWM_B` -> 驱动器 PWM 输入 B
- `PB12` -> `BIN1` -> 驱动器方向输入 B1
- `PB13` -> `BIN2` -> 驱动器方向输入 B2

#### 驱动器公共控制

- `PA4` -> `DRV_EN` -> 驱动器使能或待机控制脚

## 5.2 编码器模块

代码中编码器同样分成 A/B 两路。

#### 编码器 A

- `PB6` -> `ENC_A_CH1` -> `TIM4_CH1`
- `PB7` -> `ENC_A_CH2` -> `TIM4_CH2`

#### 编码器 B

- `PA15` -> `ENC_B_CH1` -> `TIM2_CH1`
- `PB3` -> `ENC_B_CH2` -> `TIM2_CH2`

注意：

- 编码器 B 不是走 `PA0/PA1`
- 代码里对 `TIM2` 做了部分重映射，实际使用的是 `PA15/PB3`

## 5.3 IMU 模块

代码中 IMU 驱动为 `MPU6050`，通信方式是软件 I2C：

- `PB10` -> `IMU_SCL`
- `PB11` -> `IMU_SDA`

当前代码里没有使用独立 `INT` 中断脚。

## 5.4 电池采样

- `PA6` -> `BAT_ADC` -> `ADC1_IN6`

这是代码里唯一明确配置的电压采样输入。

## 5.5 急停输入

- `PA5` -> `ESTOP`

该引脚在代码里配置为上拉输入。

## 5.6 调试串口

- `PA9` -> `USART1_TX`
- `PA10` -> `USART1_RX`

串口参数：

- `115200`
- `8N1`

接收路径使用 `USART1 + DMA1_Channel5`。

## 6. 代码中的模块对应关系

从初始化代码可以确认：

- `MotorDriver_Init(... TIM_CHANNEL_4, TIM_CHANNEL_3, ...)`
- `EncoderReader_Init(... &htim4, &htim2)`

因此当前代码内部对应关系为：

- 电机 A -> `TIM3_CH4/PB1` + 编码器 A -> `TIM4/PB6/PB7`
- 电机 B -> `TIM3_CH3/PB0` + 编码器 B -> `TIM2/PA15/PB3`

这里能确认的是 `A/B` 对应关系，**不能仅凭代码直接确认 A 是左轮还是右轮**。

## 7. 代码版接线摘要

```text
PA4   -> DRV_EN
PA5   -> ESTOP
PA6   -> BAT_ADC
PA9   -> USART1_TX
PA10  -> USART1_RX

PB0   -> PWM_B (TIM3_CH3)
PB1   -> PWM_A (TIM3_CH4)
PB12  -> BIN1
PB13  -> BIN2
PB14  -> AIN1
PB15  -> AIN2

PA15  -> ENC_B_CH1 (TIM2_CH1 remap)
PB3   -> ENC_B_CH2 (TIM2_CH2 remap)
PB6   -> ENC_A_CH1 (TIM4_CH1)
PB7   -> ENC_A_CH2 (TIM4_CH2)

PB10  -> IMU_SCL
PB11  -> IMU_SDA

PA13  -> SWDIO
PA14  -> SWCLK
PD0   -> HSE_IN
PD1   -> HSE_OUT
```

## 8. 与错误图片版的区别

本次最大的差异点是：

- 电池采样是 `PA6`，不是 `PA4`
- `PA4` 在代码里是 `DRV_EN`
- 编码器 B 使用 `PA15/PB3` 的 `TIM2` 重映射，不是旧图里的 `PA0/PA1`
- 当前代码并未启用旧图里那些扩展外设接线

因此，后续任何硬件或软件讨论，都应优先以本文档作为当前版本接线依据。
