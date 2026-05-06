# 技能配表系统 — 待办事项

> 创建日期: 2026-05-06

---

## P0 — 阻塞性问题

- [ ] **Buff GE 修饰器未注入** — `MakeBuffSpec` 接受 `EvaluatedModifierValue` 但未通过 `SetSetByCallerMagnitude` 注入到 GE Spec。当前 Buff GE 只设置持续时间，不修改目标属性。需要在 `MakeBuffSpec` 中添加 `Spec.Data->SetSetByCallerMagnitude(BuffRow.ModifierAttribute, EvaluatedModifierValue)`。
- [ ] **Subsystem DataTable 引用未配置** — `USkillConfigSubsystem` 的 6 个 UPROPERTY（3 个 DataTable + 3 个 GE 类）需要在蓝图默认值或 DefaultEngine.ini 中显式配置，否则运行时不加载任何数据。

---

## P1 — 核心功能缺失

- [ ] **冷却系统** — `CooldownFormula` 字段已定义，CSV 已填值，`UGA_DataDriven` 和 `OnAttackHit` 中均未施加冷却 GE。需要在技能激活时创建 Cooldown GE 并应用到自身 ASC。
- [ ] **消耗系统** — `CostType` / `CostFormula` 已定义，但没有任何代码在释放技能前检查并扣除资源（HP/MP/Energy）。需要读取 `CostType` Tag → 查对应 AttributeSet → 对比 `EvaluateFormula(CostFormula)` → 不足则取消释放 → 足够则扣除。
- [ ] **MakeEffectSpec 不支持 Duration** — 持续时间字段（`Duration`/`Interval`）已定义，但 `MakeEffectSpec` 仅处理即时伤害/治疗。HoT/DoT 类型效果需要设置 `Spec.Data->SetDuration()`。
- [ ] **MaxStacks 未应用到 Buff GE** — `FBuffConfigRow::MaxStacks` 已定义和配置，但 `MakeBuffSpec` 未将其应用到 GE Spec 的叠加规则中。

---

## P2 — 架构改进

- [ ] **技能等级系统** — 当前 `SkillLevel` 从 `GetAbilityLevel()` 读取（UGA_DataDriven），但 `OnAttackHit` 中硬编码 `Level=1`。需要统一等级来源：通过 ASC 查询活跃的技能 Spec 获取等级，或从外部传入。
- [ ] **Target_Area 未实现** — `Target.Area` 标签已定义，但无对应范围伤害逻辑。需要配合碰撞检测/范围查询实现。
- [ ] **UGA_DataDriven 不适合近战技能** — `UGA_DataDriven::ActivateAbility` 在遍历完效果后立即 `EndAbility`，无法在动画播放期间保持激活状态。近战技能应继续使用自定义 GA 蓝图 + `OnAttackHit` 路径；UGA_DataDriven 仅用于远程/法术类技能。
- [ ] **DataTable 改为异步加载** — 当前 `EnsureTablesLoaded` 使用 `LoadSynchronous`，首次查询时可能造成帧卡顿。建议改为 `RequestAsyncLoad` + 回调。
- [ ] **DataTable 重载支持** — 运行时 CSV 重新导入后，Subsystem 不会自动刷新缓存。需要监听 DataTable 变更事件或提供手动刷新接口。

---

## P3 — 体验优化

- [ ] **公式解析错误提示** — 畸形公式（如 `"10 +"`）静默返回 0，策划不知道配错了。建议加 `UE_LOG(Warning, ...)` 输出解析错误位置。
- [ ] **配置验证工具** — 编辑器启动时自动扫描：SkillID 关联的效果是否存在、BuffID 是否有效、公式是否可解析。
- [ ] **详情面板信息** — 在 `Widget_OptionsDetailView` 或独立调试面板中显示当前激活技能的配置信息。
- [ ] **编辑器 Attribute 约束** — `FSkillConfigRow::SkillType` 等 GameplayTag 字段已有 `meta=(Categories=...)`，但 CSV 导入时不校验 Tag 是否合法。

---

## 技术债

- [ ] `BaseCharacter::OnAttackHit` 中 `HasMatchingGameplayTag` 依赖蓝图 GA 正确配置 `ActivationOwnedTags`，否则 Subsystem 路径永不生效。需要在文档/培训中明确此前提。
- [ ] `MakeEffectSpec` 的 `TargetASC` 参数未被函数体使用，调用方自行处理应用。考虑移除该参数或让函数内部完成 Apply。
- [ ] 单元测试仅覆盖公式求值器，Subsystem 和 GA 的行为测试需要通过 UE Automation 框架补充。
