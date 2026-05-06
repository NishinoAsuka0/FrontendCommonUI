# 技能配表系统设计文档

> **创建日期**: 2026-05-06
> **状态**: 待评审

---

## 一、目标

建立策划可独立配置的技能数据管线：CSV 配表 → UE DataTable → 运行时 Subsystem 查询，让伤害/治疗/Buff 等数值效果完全表格驱动，同时保留特殊逻辑（位移、弹反等）用 C++/蓝图实现 + 表格注入参数的混合架构。

---

## 二、整体架构

```
CSV 表格层（策划编辑）
  SkillConfig.csv  |  SkillEffect.csv  |  BuffConfig.csv
        ↓
UE DataTable（导入）
  DT_SkillConfig    |  DT_SkillEffect    |  DT_BuffConfig
        ↓
USkillConfigSubsystem（GameInstance 单例）
  - 启动时加载 DataTable → TMap 缓存
  - 对外提供查询接口、公式求值、GE Spec 创建
        ↓
  ┌─────────────────┼─────────────────┐
  ▼                 ▼                  ▼
C++/蓝图 GA 类    GA_DataDriven       AbilitySet
(特殊逻辑+读表)   (通用技能全表格驱动)  (已有,不变)
```

---

## 三、数据表结构

### 3.1 SkillConfig（技能定义表）

| 列名 | 类型 | 说明 |
|------|------|------|
| SkillID | FName | 唯一标识 |
| DisplayName | FText | 显示名称 |
| SkillType | FGameplayTag | 类型标签（Skill.Melee, Skill.Ranged 等） |
| Icon | TSoftObjectPtr<UTexture2D> | 技能图标 |
| MaxLevel | int32 | 最大等级 |
| CooldownFormula | FString | 冷却公式，如 "5.0 - 0.3 * Level" |
| CostType | FGameplayTag | Cost.HP / Cost.MP / Cost.Energy |
| CostFormula | FString | 消耗公式 |
| CastRange | float | 施法距离（cm） |
| CastTime | float | 施法时间（秒） |
| AbilityTags | FGameplayTagContainer | 技能激活标签 |
| GrantToASC | bool | 是否自动授予 ASC |

### 3.2 SkillEffect（效果表）

| 列名 | 类型 | 说明 |
|------|------|------|
| EffectID | FName | 效果唯一标识 |
| SkillID | FName | 所属技能，关联 SkillConfig |
| EffectType | FGameplayTag | Effect.Damage / Effect.Heal / Effect.Buff |
| TargetType | FGameplayTag | Target.Self / Target.Enemy / Target.Area |
| BaseValue | float | 基础值 |
| ValueScale | float | 每级成长系数 |
| ValueAttribute | FGameplayTag | 修改的属性（SetByCaller Tag） |
| Duration | float | 持续时间（0=即时） |
| Interval | float | Tick 间隔 |
| BuffID | FName | 关联 BuffConfig（EffectType=Buff 时） |

### 3.3 BuffConfig（Buff/Debuff 表）

| 列名 | 类型 | 说明 |
|------|------|------|
| BuffID | FName | 唯一标识 |
| DisplayName | FText | 显示名称 |
| DurationFormula | FString | 持续公式 |
| MaxStacks | int32 | 最大叠加层数 |
| ModifierOp | EGameplayModOp | Add / Multiply / Override |
| ModifierAttribute | FGameplayTag | 修改的属性 |
| ModifierFormula | FString | 每层修改量公式 |
| Icon | TSoftObjectPtr<UTexture2D> | 状态图标 |

### 3.4 公式语法

- 变量：Level（技能等级）、Stack（Buff 层数）
- 运算符：+ - * / ( )
- 示例：`"10.0 + Level * 5.0"` `"50.0"`

由 USkillConfigSubsystem 解析并求值。

---

## 四、运行时子系统

### 4.1 USkillConfigSubsystem（GameInstanceSubsystem）

- **生命周期**：跨关卡，随 GameInstance 创建/销毁
- **初始化**：Initialize() 中加载 DT_SkillConfig / DT_SkillEffect / DT_BuffConfig
- **缓存**：TMap<FName, Row> 扁平缓存 + TMap<FName, TArray<FName>> 技能→效果映射
- **对外接口**：
  - GetSkillConfig(SkillID, Level)
  - GetSkillEffects(SkillID)
  - GetBuffConfig(BuffID, Stacks)
  - EvaluateFormula(Formula, Level, Stacks)
  - MakeEffectSpec(EffectID, Level, Source, Target)

### 4.2 UGA_DataDriven（通用技能 GameplayAbility）

- 从 DynamicAbilityTags 提取 SkillID
- ActivateAbility 流程：检查消耗 → 施加冷却 → 遍历效果 → EndAbility
- 效果分发：
  - Effect.Damage → 创建伤害 GE（SetByCaller 注入数值），应用到目标 ASC
  - Effect.Heal → 创建治疗 GE，应用到自身/目标
  - Effect.Buff → 创建 Buff GE（Modifier + Duration），应用到目标
  - 其他 → 触发 GameplayEvent，由自定义 GA 处理

### 4.3 公式求值器

- 简单递归下降解析器，支持四则运算 + 括号
- 变量替换：Level → 当前技能等级，Stack → 当前 Buff 层数
- 性能说明：每次激活技能求值 N 次（N=效果数），非每帧调用

---

## 五、与现有系统的集成

### 5.1 OnAttackHit 改造

轻/重攻击保留现有 GA 蓝图 + 动画 + 碰撞检测，仅 `OnAttackHit` 伤害来源改为读 Subsystem：

```
改造前: 硬编码 Damage = 20.0
改造后: SkillID = "LightAttack" → Subsystem.GetSkillEffects → EvaluateFormula → Damage
```

### 5.2 UAbilitySet 不变

仍配置 GrantedAbilities / GrantedEffects / GrantedAttributes，GA 类可以是 UGA_DataDriven 或自定义类，两者并存。

### 5.3 标签系统扩展

新增 GameplayTag 命名空间：
- `Skill.{SkillID}` — 技能激活标签
- `Effect.Damage` / `Effect.Heal` / `Effect.Buff` — 效果类型
- `Target.Self` / `Target.Enemy` / `Target.Area` — 目标类型
- `Cost.HP` / `Cost.MP` / `Cost.Energy` — 消耗类型
- `Attr.*` — 属性标签
- `Cooldown.*` — 冷却标签

---

## 六、需要新增/修改的文件

### 新增

| 文件 | 说明 |
|------|------|
| Public/AbilitySystem/SkillConfigTypes.h | FSkillConfigRow, FSkillEffectRow, FBuffConfigRow |
| Public/AbilitySystem/SkillConfigSubsystem.h | USkillConfigSubsystem |
| Public/AbilitySystem/SkillGameplayAbility.h | UGA_DataDriven |
| Private/AbilitySystem/SkillConfigSubsystem.cpp | 实现 |
| Private/AbilitySystem/SkillGameplayAbility.cpp | 实现 |
| Content/DataTables/DT_SkillConfig.uasset | 技能配置表 |
| Content/DataTables/DT_SkillEffect.uasset | 技能效果表 |
| Content/DataTables/DT_BuffConfig.uasset | Buff 配置表 |

### 修改

| 文件 | 改动 |
|------|------|
| Private/Character/BaseCharacter.cpp | OnAttackHit 改为读 Subsystem |
| Private/FrontendGamePlayTags.cpp | 新增 Skill/Effect/Cost/Attr Tag |
| Config/DefaultGameplayTags.ini | 同步新增 Tag |

---

## 七、实施步骤

1. 创建 SkillConfigTypes.h — 三个 FTableRow struct + 公式求值器
2. 注册 GameplayTags — Skill/Effect/Cost/Attr 命名空间
3. 创建 USkillConfigSubsystem — 加载/缓存/查询
4. 创建 UGA_DataDriven — 通用技能执行
5. 改造 OnAttackHit — 从 Subsystem 读伤害值
6. 准备 CSV 模板 + 导入 DT
7. 验证：改 CSV → 重新导入 → 伤害变化
