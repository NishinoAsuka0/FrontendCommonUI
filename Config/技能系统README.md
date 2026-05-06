# 技能配表系统 — 使用说明

> 创建日期: 2026-05-06

---

## 一、系统概述

本系统允许策划通过 **CSV 表格** 配置技能的伤害值、效果类型、目标类型、Buff 参数等数值，导入 UE DataTable 后由运行时子系统查询，实现 **数值驱动的技能管线**。

**核心原则**：通用数值效果（伤害/治疗/Buff）完全表格驱动；特殊逻辑（位移、弹反等）仍用 C++/蓝图 GA 类实现，但通过子系统读取表格中的配置参数。

---

## 二、架构速览

```
CSV 表格（策划编辑）
  ↓ 导入 UE 编辑器
DataTable（DT_SkillConfig / DT_SkillEffect / DT_BuffConfig）
  ↓ 懒加载
USkillConfigSubsystem（GameInstance 单例，跨关卡存活）
  ↓ 查询接口
┌─ 通用技能 UGA_DataDriven（全表格驱动）
└─ 自定义 GA 类（读表取数值，C++/蓝图写逻辑）
```

---

## 三、文件清单

| 文件 | 用途 |
|------|------|
| `Source/FrontendUI/Public/AbilitySystem/SkillConfigTypes.h` | 三个 FTableRow 结构体 + FFormulaEvaluator 公式求值器 |
| `Source/FrontendUI/Public/AbilitySystem/SkillConfigSubsystem.h` | 运行时子系统接口 |
| `Source/FrontendUI/Private/AbilitySystem/SkillConfigSubsystem.cpp` | 子系统实现（加载/缓存/查询/GE 创建） |
| `Source/FrontendUI/Public/AbilitySystem/SkillGameplayAbility.h` | 通用数据驱动 GA 类 |
| `Source/FrontendUI/Private/AbilitySystem/SkillGameplayAbility.cpp` | 通用 GA 实现（效果分发） |
| `Content/DataTables/DT_SkillConfig.csv` | 技能定义表模板 |
| `Content/DataTables/DT_SkillEffect.csv` | 技能效果表模板 |
| `Content/DataTables/DT_BuffConfig.csv` | Buff 配置表模板 |

---

## 四、策划工作流

### 4.1 新增一个技能

1. **编辑 CSV** — 在 `DT_SkillConfig.csv` 中添加一行

```csv
FireSlash,"(Tag=Skill.Melee)",NSLOCTEXT("","","火焰斩"),Icon_FireSlash,5,3.0 - 0.2 * Level,"(Tag=Cost.MP)",10.0 + 5.0 * Level,300.0,0.5,"(Tags=Skill.FireSlash)",true
```

2. **编辑效果** — 在 `DT_SkillEffect.csv` 中添加效果行

```csv
FireSlash_Damage,FireSlash,"(Tag=Effect.Damage)","(Tag=Target.Enemy)",30.0,8.0,"(Tag=Attr.HP)",0.0,0.0,
FireSlash_Burn,FireSlash,"(Tag=Effect.Buff)","(Tag=Target.Enemy)",0.0,0.0,"(Tag=Attr.HP)",5.0,1.0,Bleed
```

3. **导入 UE** — 编辑器中右键 CSV → Import → 选择对应 Row Struct
   - `DT_SkillConfig` → `FSkillConfigRow`
   - `DT_SkillEffect` → `FSkillEffectRow`
   - `DT_BuffConfig` → `FBuffConfigRow`

4. **配置子系统引用** — 在 Project Settings 或 Subsystem 蓝图默认值中设置：
   - `SkillConfigTable` → `DT_SkillConfig`
   - `SkillEffectTable` → `DT_SkillEffect`
   - `BuffConfigTable` → `DT_BuffConfig`
   - `DamageGEClass` → 通用伤害 GE 蓝图
   - `HealGEClass` → 通用治疗 GE 蓝图
   - `BuffGEClass` → 通用 Buff GE 蓝图

5. **测试** — 运行游戏，触发技能，验证伤害/效果符合预期

### 4.2 修改技能数值

只需修改 CSV → 重新导入 → 重启 PIE 测试。**不需要改任何 C++ 代码。**

---

## 五、表格列说明

### DT_SkillConfig（技能定义）

| 列名 | 类型 | 说明 | 示例 |
|------|------|------|------|
| Name | FName | 行名=技能ID | `LightAttack` |
| SkillType | FGameplayTag | 类型标签 | `(Tag=Skill.LightAttack)` |
| DisplayName | FText | 显示名 | `NSLOCTEXT("","","轻攻击")` |
| Icon | TSoftObjectPtr | 图标（空=无） | 留空 |
| MaxLevel | int32 | 最大等级 | `5` |
| CooldownFormula | FString | 冷却公式 | `0.5` |
| CostType | FGameplayTag | 消耗类型 | `(Tag=Cost.Energy)` |
| CostFormula | FString | 消耗公式 | `5.0` |
| CastRange | float | 施法距离(cm) | `150.0` |
| CastTime | float | 施法时间(秒) | `0.0` |
| AbilityTags | FGameplayTagContainer | 激活标签 | `(Tags=Ability.Melee.Light)` |
| bGrantToASC | bool | 是否自动授予ASC | `true` |

### DT_SkillEffect（技能效果）

| 列名 | 类型 | 说明 |
|------|------|------|
| Name | FName | 行名=效果ID |
| SkillID | FName | 所属技能ID（关联SkillConfig行名） |
| EffectType | FGameplayTag | `Effect.Damage` / `Effect.Heal` / `Effect.Buff` |
| TargetType | FGameplayTag | `Target.Self` / `Target.Enemy` / `Target.Area` |
| BaseValue | float | 等级1时基础值 |
| ValueScale | float | 每级成长系数 |
| ValueAttribute | FGameplayTag | SetByCaller属性，如 `Attr.HP` |
| Duration | float | 持续时间（0=即时） |
| Interval | float | Tick间隔（0=无Tick） |
| BuffID | FName | 关联BuffConfig（EffectType=Buff时必填） |

### DT_BuffConfig（Buff/Debuff）

| 列名 | 类型 | 说明 |
|------|------|------|
| Name | FName | BuffID |
| DisplayName | FText | 显示名 |
| DurationFormula | FString | 持续公式 |
| MaxStacks | int32 | 最大层数 |
| ModifierOp | EGameplayModOp | Additive/Multiply/Override |
| ModifierAttribute | FGameplayTag | 修改属性 |
| ModifierFormula | FString | 每层修改量公式 |
| Icon | TSoftObjectPtr | 图标 |

---

## 六、公式语法

公式字符串支持 **四则运算 + 括号 + 变量**：

| 变量 | 含义 |
|------|------|
| `Level` | 当前技能等级 |
| `Stack` | 当前 Buff 层数 |

**示例**：
- `"50.0"` — 固定值 50
- `"10.0 + 5.0 * Level"` — Level=3 → 25.0
- `"3.0 + 1.0 * Level"` — Level=2 → 5.0
- `"(10 + Level) * (2 + Stack)"` — Level=2, Stack=3 → 60.0

---

## 七、GameplayTags 命名空间

| 命名空间 | 用途 | 示例 |
|---------|------|------|
| `Skill.*` | 技能标识 | `Skill.LightAttack`, `Skill.FireSlash` |
| `Effect.*` | 效果类型 | `Effect.Damage`, `Effect.Heal`, `Effect.Buff` |
| `Target.*` | 目标类型 | `Target.Self`, `Target.Enemy`, `Target.Area` |
| `Cost.*` | 消耗资源 | `Cost.HP`, `Cost.MP`, `Cost.Energy` |
| `Attr.*` | 属性标识 | `Attr.HP`, `Attr.MP`, `Attr.Energy` |

---

## 八、API 速查

```cpp
// 获取子系统
USkillConfigSubsystem* SS = USkillConfigSubsystem::Get(this);

// 查询技能配置
const FSkillConfigRow* Cfg = SS->GetSkillConfig("LightAttack");

// 查询技能效果
TArray<FSkillEffectRow> Effects = SS->GetSkillEffects("LightAttack");

// 查询 Buff 配置
const FBuffConfigRow* Buff = SS->GetBuffConfig("Bleed");

// 公式求值
float Damage = SS->EvaluateFormula("15.0 + 5.0 * Level", 3); // Level=3 → 30.0

// 创建伤害 GE Spec
FGameplayEffectSpecHandle Spec = SS->MakeEffectSpec(EffectRow, Damage, this, TargetASC);
```

---

## 九、如何接入已有的轻/重攻击

轻/重攻击保留现有 GA 蓝图（动画 + 碰撞检测），伤害来源已改为从 Subsystem 读取：

- `BaseCharacter.h` 中配置 `LightAttackSkillID` / `HeavyAttackSkillID`（默认 `LightAttack` / `HeavyAttack`）
- CSV 中确保有对应的 `Name=LightAttack` 行和关联效果
- 武器命中时自动查表获取伤害值，Subsystem 不可用时降级为旧硬编码逻辑
