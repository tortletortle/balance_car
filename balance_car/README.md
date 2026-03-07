# balance_car

当前仓库已完成整理阶段，现进入“按分层架构重写平衡小车”的驱动层开发阶段。

## 当前阶段目标

- 固定板级硬件事实、工具链和工程边界
- 按 `board -> drivers -> devices -> modules -> control -> app` 分层推进重写
- 先完成可讲解、可维护的基础驱动，再进入器件、模块和控制层
- 每完成一块，先更新文档，再提交 Git

## 参考项目

- 第一完成版参考工程：`D:\program\learning\balance_v3`

该工程当前仅作为参考资产，不能直接视为新项目的实现基础。

## 当前文档

- `docs/PROJECT_UNDERSTANDING.md`：对 `balance_v3` 的结构化理解
- `docs/WORKFLOW.md`：当前阶段的工作方式、提交与记录规则
- `docs/DECISIONS.md`：阶段性决策记录
- `docs/CHANGELOG.md`：每次整理或实现动作的变更日志

## 当前约束

- 先整理，再设计，再开发
- 每完成一块，先更新文档，再提交 Git
- 提交保持小而清晰，禁止把无关旧资料一起纳入版本历史
