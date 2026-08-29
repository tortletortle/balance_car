# 变更记录

## 2026-03-12

### [调试] 角度环参数回调

- 将默认 `angle_loop_config.kp_q8` 从 `16384` 下调到 `14336`
- 将默认 `angle_loop_config.kd_q8` 从 `1792` 提高到 `2048`
- 将默认 `angle_loop_config.kp_q8` 从 `14336` 下调到 `13312`
- 将默认 `angle_loop_config.kd_q8` 从 `2048` 提高到 `2304`
- 将默认 `angle_loop_config.kd_q8` 从 `2304` 提高到 `2560`
- 将默认 `angle_loop_config.kp_q8` 从 `13312` 提高到 `14336`
- 将默认 `angle_loop_config.kp_q8` 从 `14336` 提高到 `32767`
- 将默认 `angle_loop_config.kp_q8` 从 `32767` 下调到 `28672`
- 将默认 `angle_loop_config.kp_q8` 从 `28672` 下调到 `24576`
- 将默认 `angle_loop_config.kp_q8` 从 `24576` 下调到 `23040`
- 将默认 `angle_loop_config.kp_q8` 从 `23040` 下调到 `20480`
- 将默认 `angle_loop_config.kd_q8` 从 `2560` 提高到 `3200`

### [调试] PWM 上限提高

- 将 `motor_pwm_limit` 从 `6500` 提高到 `7000`
- 将 `speed_pwm_limit` 从 `6500` 提高到 `7000`

### [调试] 方向补偿微调

- 将默认 `motor_dir_scale_rev_x1000` 从 `1050` 提高到 `1100`
- 将默认 `motor_dir_scale_rev_x1000` 从 `1100` 提高到 `1150`
- 将默认 `motor_dir_scale_rev_x1000` 从 `1150` 下调到 `1125`

### [调试] 小角度补推加强

- 将 `assist_min_pwm_1` 从 `200` 提高到 `260`
- 将 `assist_min_pwm_2` 从 `350` 提高到 `420`
- 将 `assist_min_cmd_1` 从 `40` 下调到 `30`
- 将 `assist_min_pwm_1` 从 `260` 提高到 `300`
- 将 `assist_min_pwm_2` 从 `420` 提高到 `480`
- 将 `assist_min_cmd_1` 从 `30` 下调到 `20`
- 将 `assist_min_pwm_1` 从 `300` 提高到 `340`
- 将 `assist_min_pwm_2` 从 `480` 提高到 `520`
- 将 `assist_min_cmd_1` 从 `20` 下调到 `15`
- 将 `assist_min_pwm_1` 从 `340` 提高到 `380`
- 将 `assist_min_pwm_2` 从 `520` 提高到 `560`
- 将 `assist_min_cmd_1` 从 `15` 下调到 `10`
- 将 `assist_min_pwm_1` 从 `380` 提高到 `420`
- 将 `assist_min_pwm_2` 从 `560` 提高到 `600`
- 将 `assist_min_cmd_1` 从 `10` 下调到 `8`
- 将 `assist_min_pwm_1` 从 `420` 提高到 `460`
- 将 `assist_min_pwm_2` 从 `600` 提高到 `650`
- 将 `assist_min_cmd_1` 从 `8` 下调到 `6`
- 将 `assist_min_pwm_1` 从 `460` 提高到 `520`
- 将 `assist_min_pwm_2` 从 `650` 提高到 `720`
- 将 `assist_min_cmd_1` 从 `6` 下调到 `4`
- 将 `assist_min_pwm_1` 从 `520` 提高到 `600`
- 将 `assist_min_pwm_2` 从 `720` 提高到 `850`
- 将 `assist_min_cmd_1` 从 `4` 下调到 `3`
- 将 `assist_min_pwm_1` 从 `600` 提高到 `700`
- 将 `assist_min_pwm_2` 从 `850` 提高到 `1000`
- 将 `assist_min_cmd_1` 从 `3` 下调到 `2`
- 将 `assist_min_pwm_1` 从 `700` 提高到 `1200`
- 将 `assist_min_pwm_2` 从 `1000` 提高到 `1500`
- 将 `assist_min_cmd_1` 从 `2` 下调到 `1`
- 将 `assist_min_pwm_1` 从 `1200` 提高到 `1600`
- 将 `assist_min_pwm_2` 从 `1500` 提高到 `2200`
- 将 `assist_min_pwm_1` 从 `1600` 提高到 `2000`
- 将 `assist_min_pwm_2` 从 `2200` 提高到 `3000`
- 将 `assist_min_pwm_1` 从 `2000` 提高到 `2600`
- 将 `assist_min_pwm_2` 从 `3000` 提高到 `3600`
- 将 `assist_min_cmd_1` 从 `1` 提高到 `8`
- 将 `assist_min_pwm_1` 从 `2600` 下调到 `1400`
- 将 `assist_min_pwm_2` 从 `3600` 下调到 `1800`
- 将 `assist_min_cmd_1` 从 `8` 提高到 `12`
- 将 `assist_min_pwm_1` 从 `1400` 下调到 `1100`
- 将 `assist_min_pwm_2` 从 `1800` 下调到 `1500`
- 将 `assist_min_cmd_2` 从 `120` 提高到 `200`
- 将 `assist_min_pwm_2` 从 `1500` 下调到 `1200`
- 将 `deadzone_comp_min_cmd` 从 `20` 提高到 `160`
- 将 `assist_min_pwm_1` 从 `1100` 下调到 `900`
- 将 `deadzone_comp_min_cmd` 从 `160` 提高到 `200`
- 将默认 `angle_loop_config.kp_q8` 从 `20480` 下调到 `18432`
- 将默认 `angle_loop_config.kp_q8` 从 `18432` 提高到 `20480`
- 将默认 `angle_loop_config.cmd_limit` 从 `6500` 提高到 `7000`
- 将 `assist_min_pwm_2` 从 `650` 提高到 `720`

### [调参] 降低 Kp、提升 Kd、收敛指令上限

- 将默认 `angle_loop_config.kp_q8` 从 `20480` 下调到 `19200`
- 将默认 `angle_loop_config.kd_q8` 从 `3200` 提高到 `3840`
- 将默认 `angle_loop_config.cmd_limit` 从 `7000` 下调到 `6500`
- 将默认 `deadzone_comp_min_cmd` 从 `200` 提高到 `300`
- 将 `assist_full_cmd` 从 `600` 提高到 `900`
- 将 `assist_full_cmd` 从 `900` 提高到 `1200`
- 将默认 `angle_loop_config.kp_q8` 从 `19200` 下调到 `17600`
- 将默认 `angle_loop_config.kd_q8` 从 `3840` 提高到 `4480`
- 将默认 `angle_loop_config.cmd_limit` 从 `6500` 下调到 `6000`
- 将默认 `angle_loop_config.kp_q8` 从 `17600` 下调到 `16000`
- 将默认 `angle_loop_config.kd_q8` 从 `4480` 提高到 `5120`
- 将默认 `angle_loop_config.cmd_limit` 从 `6000` 下调到 `5000`
- 将默认 `angle_loop_config.kp_q8` 从 `16000` 下调到 `15000`
- 将默认 `angle_loop_config.kd_q8` 从 `5120` 提高到 `5760`
- 将默认 `angle_loop_config.cmd_limit` 从 `5000` 下调到 `4500`
- 将默认 `angle_loop_config.kp_q8` 从 `15000` 下调到 `14000`
- 将默认 `angle_loop_config.kd_q8` 从 `5760` 提高到 `6400`
- 将默认 `angle_loop_config.cmd_limit` 从 `4500` 下调到 `4000`
- 将默认 `angle_loop_config.kp_q8` 从 `14000` 下调到 `13000`
- 将默认 `angle_loop_config.kd_q8` 从 `6400` 提高到 `7040`
- 将默认 `angle_loop_config.cmd_limit` 从 `4000` 下调到 `3500`
- 将默认 `angle_loop_config.kp_q8` 从 `13000` 下调到 `12000`
- 将默认 `angle_loop_config.kd_q8` 从 `7040` 提高到 `7680`
- 将默认 `angle_loop_config.cmd_limit` 从 `3500` 下调到 `3000`
- 将默认 `angle_loop_config.kp_q8` 从 `12000` 下调到 `10500`
- 将默认 `angle_loop_config.kd_q8` 从 `7680` 提高到 `8960`
- 将默认 `angle_loop_config.cmd_limit` 从 `3000` 下调到 `2500`
- 将默认 `angle_loop_config.kp_q8` 从 `10500` 下调到 `9500`
- 将默认 `angle_loop_config.kd_q8` 从 `8960` 提高到 `10240`
- 将默认 `angle_loop_config.cmd_limit` 从 `2500` 下调到 `2000`
- 将默认 `angle_loop_config.kp_q8` 从 `9500` 提高到 `10500`
- 将默认 `angle_loop_config.kd_q8` 从 `10240` 提高到 `11520`
- 将默认 `angle_loop_config.kp_q8` 从 `10500` 提高到 `11500`
- 将默认 `angle_loop_config.kd_q8` 从 `11520` 下调到 `10752`
- 将默认 `deadzone_comp_min_cmd` 从 `300` 下调到 `200`
- 将默认 `deadzone_comp_min_cmd` 从 `200` 下调到 `150`
- 将默认 `deadzone_comp_min_cmd` 从 `150` 下调到 `80`
- 将默认 `deadzone_comp_min_cmd` 从 `80` 下调到 `40`
- 将默认 `deadzone_comp_min_cmd` 从 `40` 下调到 `10`
- 将启动助推 `assist_min_pwm_1` 从 `900` 提高到 `1000`
- 将启动助推 `assist_min_pwm_2` 从 `1200` 提高到 `1300`
- 将启动助推 `assist_min_pwm_1` 从 `1000` 提高到 `1100`
- 将启动助推 `assist_min_pwm_2` 从 `1300` 提高到 `1400`
- 将启动助推 `assist_min_pwm_1` 从 `1100` 提高到 `1200`
- 将启动助推 `assist_min_pwm_2` 从 `1400` 提高到 `1500`
- 将默认 `deadzone_comp_min_cmd` 从 `10` 提高到 `80`
- 将启动助推 `assist_min_pwm_1` 从 `1200` 下调到 `900`
- 将启动助推 `assist_min_pwm_2` 从 `1500` 下调到 `1200`
- 将默认 `motor_dir_scale_rev_x1000` 从 `1125` 提高到 `1150`
- 将默认 `motor_dir_scale_rev_x1000` 从 `1150` 提高到 `1175`
- 将默认 `angle_loop_config.kd_q8` 从 `10752` 提高到 `12800`
- 将默认 `angle_loop_config.cmd_limit` 从 `2000` 提高到 `2600`
- 将默认 `angle_loop_config.kp_q8` 从 `11500` 提高到 `13000`
- 将默认 `angle_loop_config.kd_q8` 从 `12800` 提高到 `14080`
- 将默认 `angle_loop_config.cmd_limit` 从 `2600` 提高到 `3200`
- 将默认 `initial_target_pitch_mdeg` 从 `0` 调整为 `300`
- 将默认 `initial_target_pitch_mdeg` 从 `300` 调整为 `-300`
- 将默认 `initial_target_pitch_mdeg` 从 `-300` 调整回 `0`

## 2026-03-11

### [调试] 开启速度闭环（轮子动力差测试）

- 将默认 `speed_enable_closed_loop` 调整为 `1`
- 保持 `speed_cmd_to_delta_div=100`，仅用于观察轮速闭环响应

### [调试] STATUS 增加轮子转速输出

- `STATUS` 追加 `EDA/EDB` 字段，输出两侧编码器每控制周期的增量
- 便于在串口对比左右轮实际转速差异

### [调试] 增加左右轮比例补偿

- 新增 `motor_scale_a_x1000/motor_scale_b_x1000`，用于修正左右轮速度偏差
- 默认设置为 `A=1000`、`B=850`（B 侧下调约 15%）

### [调试] 提高角度环 Kp

- 将默认 `angle_loop_config.kp_q8` 从 `16384` 提高到 `18432`
- 将默认 `angle_loop_config.kp_q8` 从 `18432` 提高到 `20480`
- 将默认 `angle_loop_config.kp_q8` 从 `20480` 提高到 `22528`
- 将默认 `angle_loop_config.kp_q8` 从 `22528` 提高到 `24576`
- 将默认 `angle_loop_config.kp_q8` 从 `24576` 提高到 `26624`

### [调试] 提高角度环 Kd

- 将默认 `angle_loop_config.kd_q8` 从 `256` 提高到 `512`

### [调试] 开机自动 ARM

- `app_reset()` 默认将 `arm_request` 设为 `1`

### [调试] 提升小角度启动补推

- 将默认 `deadzone_comp_min_cmd` 从 `40` 下调到 `20`
- 将小角度补推下限 `assist_min_pwm_1/2` 从 `600/900` 提高到 `800/1200`
- 将默认 `deadzone_comp_min_cmd` 从 `20` 下调到 `10`

### [调试] 提升小角度分辨率与力度

- 角度环 `ddeg` 由 `mdeg/100` 调整为 `mdeg/50`（0.05° 分辨率）
- 同步将角速度差分缩放由 `/100000` 调整为 `/50000`
- 默认 `angle_loop_config.kp_q8` 调整为 `20480`
- 默认 `angle_loop_config.d_input_limit_ddeg` 调整为 `800`

### [调试] 降低补推阶跃并加强阻尼

- 启动补推最小 PWM 由 `900/1400` 下调到 `600/900`
- `deadzone_comp_min_cmd` 调整为 `20`
- 默认 `angle_loop_config.kd_q8` 调整为 `768`

### [调试] 进一步降低小角度补推跳变

- 启动补推最小 PWM 调整为 `200/350`
- 停转强化触发周期从 `2` 提高到 `4`
- 默认 `motor_deadzone_pwm_fwd/rev` 调整为 `900/900`

### [调试] 增强阻尼抑制自激

- 默认 `angle_loop_config.kp_q8` 调整为 `18432`
- 默认 `angle_loop_config.kd_q8` 调整为 `1024`
- 默认 `d_gyro_weight` 调整为 `3/4`

### [调试] 小角度禁用强补推

- 只有 `abs_base_cmd >= 300` 才允许触发停转强化补推（`startup_pwm + 200`）

### [调试] 降低中等指令的补推强度

- `assist_full_cmd` 从 `300` 调整为 `600`
- 停转强化补推改为 `abs_base_cmd >= 600` 才触发

### [调试] 提高中等角度扭矩

- 默认 `motor_deadzone_pwm_fwd/rev` 调整为 `1200/1200`

### [调试] 进一步增强阻尼

- 默认 `angle_loop_config.kd_q8` 调整为 `1536`
- 默认 `d_gyro_weight` 调整为 `7/8`

### [调试] 倾倒保护

- `|PITCH| >= 15°` 自动取消 ARM 并重置控制链路

### [调参] 降低 Kp、提升 Kd

- 默认 `angle_loop_config.kp_q8` 调整为 `16384`
- 默认 `angle_loop_config.kd_q8` 调整为 `1792`
- 将小角度补推下限 `assist_min_pwm_1/2` 从 `800/1200` 提高到 `900/1400`

### [调试] 增加方向补偿

- 新增 `motor_dir_scale_fwd_x1000/motor_dir_scale_rev_x1000`
- 默认设置为 `FWD=1000`、`REV=1100`（反向方向 +10%）

### [调试] 回退闭环与增益

- 关闭速度闭环：`speed_enable_closed_loop=0`
- 将默认 `angle_loop_config.kp_q8` 回退到 `24576`
- 方向补偿回调到 `REV=1050`

### [调试] 提升小角度分辨率

- `pitch_error_ddeg/pitch_actual_ddeg` 改为四舍五入（替代直接 `/100` 截断）

## 2026-03-10

### [调试] 提高输出上限与起转力度

- 将默认 `speed_pwm_limit/motor_pwm_limit` 从 `1780` 调整为 `3000`
- 将默认 `angle_cmd_limit` 从 `1780` 调整为 `3000`
- 将默认 `speed_pwm_limit/motor_pwm_limit` 进一步调整为 `3600`
- 将默认 `angle_cmd_limit` 进一步调整为 `3600`
- 将默认 `speed_pwm_limit/motor_pwm_limit` 进一步调整为 `4800`
- 将默认 `angle_cmd_limit` 进一步调整为 `4800`
- 将默认 `speed_pwm_limit/motor_pwm_limit` 进一步调整为 `6500`
- 将默认 `angle_cmd_limit` 进一步调整为 `6500`
- 将默认 `startup_pwm_fwd/rev` 从 `1200/1200` 调整为 `1600/1600`
- 目标是提升拉回能力，避免在 2°~4° 区域提前打满仍不足

### [调试] 轻微提高 PD

- 将默认 `angle_loop_config.kp_q8` 从 `12288` 调整为 `14336`
- 将默认 `angle_loop_config.kd_q8` 从 `0` 调整为 `256`
- 保持 `ki_q8=0` 不变，仅测试 P+D 的增益变化

### [调试] 再次提高 PD

- 将默认 `angle_loop_config.kp_q8` 从 `14336` 调整为 `16384`
- 将默认 `angle_loop_config.kd_q8` 从 `256` 调整为 `512`
- 保持 `ki_q8=0` 不变，继续验证拉回能力与抖动平衡

### [调试] 回退 D 增益

- 将默认 `angle_loop_config.kd_q8` 从 `512` 调整回 `256`
- 保持 `kp_q8=16384`、`ki_q8=0` 不变，降低过冲与饱和反向冲击

### [调试] 小角度补推改为线性缩放

- 将 `startup` 补推从“直接跳到启动 PWM”改为“随命令线性放大”
- 满补推阈值设为 `800`，小角度不再一次跳到 `startup_pwm`
- 将满补推阈值从 `800` 下调到 `300`，增强小角度响应
- 增加二段式最小补推：`40~120 -> 600`，`120~300 -> 900`

### [调试] 打开速度闭环并提高目标速度

- 将 `speed_enable_closed_loop` 从 `0` 调整为 `1`
- 将 `speed_cmd_to_delta_div` 从 `100` 调整为 `50`
- 目标是让小角度也能获得更高的实际轮速

### [调试] 再提高恢复力与执行响应

- 将默认 `angle_loop_config.kp_q8` 从 `10240` 调整为 `12288`
- 保持 `ki_q8=0`、`kd_q8=0` 不变，继续只看纯比例直立恢复能力
- 将默认 `motor_deadzone_pwm_fwd/rev` 从 `1000/1000` 调整为 `1200/1200`
- 将默认 `motor_ramp_step` 从 `600` 调整为 `1200`
- 这次修改针对实测现象：已经有明显力度，但中角度恢复仍偏软，执行链还需要更快跟手


### [调试] 提前触发小角度补推

- 将默认 `deadzone_comp_min_cmd` 从 `50` 调整为 `40`
- 目标是让约 `0.1°` 量级产生的 `ACMD` 更容易跨过启动补推门槛，改善小角度不灵敏现象
### [调试] 提高小角度灵敏度与恢复力

- 将默认 `angle_loop_config.kp_q8` 从 `7168` 调整为 `10240`
- 将默认 `angle_loop_config.cmd_limit` 从 `1500` 调整为 `1780`
- 将默认 `angle_loop_config.error_deadband_ddeg` 从 `5` 调整为 `0`，去掉 ±0.5° 内直接清零输出的问题
- 将默认 `motor_deadzone_pwm_fwd/rev` 从 `800/800` 调整为 `1000/1000`
- 将默认 `motor_ramp_step` 从 `200` 调整为 `600`，缩短从 `0` 爬升到有效驱动 PWM 的时间
- 这次修改针对实测现象：小角度 `ACMD/PWM` 归零、大角度恢复力仍不足

### [??] ?????? PWM ???????

- ??? `motor_deadzone_pwm_fwd/rev` ? `600/600` ??? `800/800`
- ? `ctrl_speed_loop` ????????????????????????????????????????????????????? PWM ??? `startup_pwm + 200`
- ??????????????????????????????????

### [调试] 调整默认角度环与启动 PWM

- 将默认 `angle_loop_config.ki_q8` 从 `16` 调整为 `0`
- 将默认 `angle_loop_config.kd_q8` 从 `256` 调整为 `0`
- 将默认 `motor_deadzone_pwm_fwd/rev` 从 `540/540` 调整为 `600/600`
- 保持 `KP=7168`、`deadzone_comp_min_cmd=50`、`motor_ramp_step=200` 不变，按本轮调车判断继续验证“恢复力不足是否主要来自启动 PWM”

### [调试] 提高默认角度环比例增益

- 将默认 `angle_loop_config.kp_q8` 从 `2816` 提高到 `7168`
- 这次修改只调整上电默认角度环比例增益，便于重启后直接进入更积极的恢复力调试
### [Docs] Add external tuning reference summary

- Add `docs/EXTERNAL_TUNING_REFERENCE.md` to summarize official references for PID tuning, IMU fusion, encoder/timer usage and TB6612 drive-chain behavior
- Translate the source material into project-facing conclusions, focusing on current balance-car debugging rather than generic theory copying
- Add the new note to `README.md` so it is easy to find during tuning and review

## 2026-03-08

### [调试] 下调低速区默认补偿参数

- 将默认 `motor_deadzone_pwm_fwd` 从 `900` 下调到 `540`
- 将默认 `motor_deadzone_pwm_rev` 从 `980` 下调到 `540`
- 将默认 `deadzone_comp_min_cmd` 从 `80` 下调到 `50`
- 将默认 `motor_ramp_step` 从 `80` 提升到 `200`
- 本轮目标是先消除零点附近“空挡感 / 过坎感 / 突然蹿一下”的低速区问题，为后续只调角度环 `P/D` 创造更真实的手感

### [调试] 修正闭环电机方向符号

- 根据架空闭环方向测试结果，确认当前整体执行方向与扶正方向相反
- 将默认 `motor_command_sign_a/b` 从 `+1/+1` 翻转为 `-1/-1`
- 保持编码器方向和速度符号不动，只修正执行链整体符号，避免混改

### [调试] 增加轮子方向 bench 点动命令

- 新增 `MOTOR <pwm>`、`MOTOR A <pwm>`、`MOTOR B <pwm>`、`MOTOR STOP` 串口命令，用于架空状态下低风险测试轮子方向
- bench 点动命令会自动取消 `ARM` 请求，避免与平衡控制输出混用
- bench 输出只受硬件输出开关、急停和欠压约束，不依赖 IMU 就绪，便于先测电机方向
- 默认打开 `board` 层真实电机输出开关，方便进入调车阶段

### [调试] 补充角度环在线调参命令

- 新增 `ANGLE` / `PID` 参数查看命令，串口可直接打印当前角度环配置
- 新增 `ANGLE KP <q8>`、`ANGLE KI <q8>`、`ANGLE KD <q8>`、`ANGLE LIMIT <cmd>`、`ANGLE DEAD <ddeg>` 在线调参命令
- 上电启动时增加 `ANGLE,CFG,...` 参数遥测，便于记录每轮调车时实际使用的内环参数
- 串口帮助文本同步更新，避免现场调车时再去翻代码找命令

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








