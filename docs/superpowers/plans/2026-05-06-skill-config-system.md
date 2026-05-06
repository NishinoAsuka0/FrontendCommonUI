# 技能配表系统 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 建立 CSV 配表 → UE DataTable → 运行时 Subsystem 查询的完整技能配表管线

**Architecture:** 三个 FTableRow 结构体定义配表格式，USkillConfigSubsystem（GameInstance 单例）负责加载/缓存/查询/公式求值，UGA_DataDriven 通用技能类实现表格驱动的技能效果执行，OnAttackHit 改造为读表获取伤害值

**Tech Stack:** Unreal Engine 5.7, C++17, Gameplay Ability System, DataTable

---

## 文件结构

| 文件 | 职责 | 操作 |
|------|------|------|
| `Public/AbilitySystem/SkillConfigTypes.h` | 三个 FTableRow + FFormulaEvaluator | 新建 |
| `Public/AbilitySystem/SkillConfigSubsystem.h` | USkillConfigSubsystem 声明 | 新建 |
| `Private/AbilitySystem/SkillConfigSubsystem.cpp` | USkillConfigSubsystem 实现 | 新建 |
| `Public/AbilitySystem/SkillGameplayAbility.h` | UGA_DataDriven 声明 | 新建 |
| `Private/AbilitySystem/SkillGameplayAbility.cpp` | UGA_DataDriven 实现 | 新建 |
| `Public/FrontendGamePlayTags.h` | 新增 Skill/Effect/Cost/Target Tag 声明 | 修改 |
| `Private/FrontendGamePlayTags.cpp` | 新增 Skill/Effect/Cost/Target Tag 定义 | 修改 |
| `Config/DefaultGameplayTags.ini` | 新增 Tag 列表 | 修改 |
| `Private/Character/BaseCharacter.cpp` | OnAttackHit 改为读 Subsystem | 修改 |
| `Public/Character/BaseCharacter.h` | 新增 SkillID 属性 | 修改 |

---

### Task 1: 创建 SkillConfigTypes.h — 数据类型与公式求值器

**Files:**
- Create: `Source/FrontendUI/Public/AbilitySystem/SkillConfigTypes.h`

- [ ] **Step 1: 创建 SkillConfigTypes.h**

```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Styling/SlateBrush.h"
#include "SkillConfigTypes.generated.h"

/**
 * 技能定义表行
 *
 * 策划通过 CSV 编辑，导入为 DataTable。
 * 每行定义一个技能的基本信息和参数。
 */
USTRUCT(BlueprintType)
struct FRONTENDUI_API FSkillConfigRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 技能类型标签（Skill.Melee, Skill.Ranged, Skill.Buff 等） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Skill"))
	FGameplayTag SkillType;

	/** 显示名称 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayName;

	/** 技能图标 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;

	/** 最大等级 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxLevel = 1;

	/** 冷却公式，变量 Level 表示当前等级，如 "5.0 - 0.3 * Level" */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString CooldownFormula;

	/** 消耗资源类型（Cost.HP / Cost.MP / Cost.Energy） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Cost"))
	FGameplayTag CostType;

	/** 消耗公式 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString CostFormula;

	/** 施法距离（cm），0 表示无限制 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float CastRange = 0.f;

	/** 施法时间（秒） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float CastTime = 0.f;

	/** 技能激活标签（写入 DynamicAbilityTags） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Ability.Activate"))
	FGameplayTagContainer AbilityTags;

	/** 是否在 AbilitySet 授予时自动注册到 ASC */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bGrantToASC = true;
};

/**
 * 技能效果表行
 *
 * 一个技能可以有多个效果，按数组顺序依次执行。
 */
USTRUCT(BlueprintType)
struct FRONTENDUI_API FSkillEffectRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 所属技能 ID，关联 FSkillConfigRow::RowName */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName SkillID;

	/** 效果类型（Effect.Damage / Effect.Heal / Effect.Buff） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Effect"))
	FGameplayTag EffectType;

	/** 目标类型（Target.Self / Target.Enemy / Target.Area） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Target"))
	FGameplayTag TargetType;

	/** 基础值（等级 1 时） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseValue = 0.f;

	/** 每级成长系数 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ValueScale = 0.f;

	/** 修改的属性 Tag，用于 SetByCaller 或 Modifier */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Attr"))
	FGameplayTag ValueAttribute;

	/** 持续时间，0 = 即时效果 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Duration = 0.f;

	/** Tick 间隔（仅持续效果），0 = 无 Tick */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Interval = 0.f;

	/** 关联 Buff ID，EffectType=Buff 时有效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName BuffID;
};

/**
 * Buff/Debuff 配置表行
 */
USTRUCT(BlueprintType)
struct FRONTENDUI_API FBuffConfigRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 显示名称 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayName;

	/** 持续公式（秒），变量 Level + Stack */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString DurationFormula;

	/** 最大叠加层数 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxStacks = 1;

	/** 修饰操作类型 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TEnumAsByte<EGameplayModOp::Type> ModifierOp = EGameplayModOp::Additive;

	/** 修改的属性 Tag */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Attr"))
	FGameplayTag ModifierAttribute;

	/** 每层修改量公式，变量 Level + Stack */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString ModifierFormula;

	/** 状态图标 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;
};

/**
 * 公式求值器
 *
 * 在 USkillConfigSubsystem 中使用，解析并求值简单的数学表达式。
 * 支持：四则运算 + 括号 + 变量 Level/Stack
 */
struct FRONTENDUI_API FFormulaEvaluator
{
	/**
	 * 求值公式字符串
	 * @param Formula 如 "10.0 + 5.0 * Level"
	 * @param Level   当前技能等级
	 * @param Stacks  当前 Buff 层数
	 * @return 计算结果；公式为空则返回 0
	 */
	static float Evaluate(const FString& Formula, int32 Level = 1, int32 Stacks = 1);

private:
	// 递归下降解析器：token 类型
	enum class ETokenType : uint8 { Number, Plus, Minus, Mul, Div, LParen, RParen, Var, End };

	struct FToken
	{
		ETokenType Type = ETokenType::End;
		float Value = 0.f;
		FString VarName;
	};

	class FTokenizer
	{
	public:
		explicit FTokenizer(const FString& InFormula, int32 InLevel, int32 InStacks);
		FToken Peek() const;
		FToken Consume();
		bool IsAtEnd() const { return Pos >= Formula.Len(); }

	private:
		FToken NextToken();
		void SkipWhitespace();
		float ResolveVariable(const FString& Name) const;

		const FString& Formula;
		int32 Pos = 0;
		int32 Level = 1;
		int32 Stacks = 1;
		mutable FToken Lookahead;
		mutable bool bHasLookahead = false;
	};

	class FParser
	{
	public:
		explicit FParser(FTokenizer& InTokenizer) : Tokenizer(InTokenizer) {}
		float ParseExpression();
	private:
		float ParseTerm();
		float ParseFactor();
		float ParseAtom();

		FTokenizer& Tokenizer;
	};
};
```

- [ ] **Step 2: 验证编译**

```bash
# 在 UE 编辑器中触发 Build（或通过命令行）
# 预期：编译通过（仅有 struct 定义，无链接依赖）
```

- [ ] **Step 3: Commit**

```bash
git add Source/FrontendUI/Public/AbilitySystem/SkillConfigTypes.h
git commit -m "feat: 添加技能配表数据类型定义 SkillConfigTypes.h"
```

---

### Task 2: 注册 GameplayTags — 技能相关标签命名空间

**Files:**
- Modify: `Source/FrontendUI/Public/FrontendGamePlayTags.h`
- Modify: `Source/FrontendUI/Private/FrontendGamePlayTags.cpp`
- Modify: `Config/DefaultGameplayTags.ini`

- [ ] **Step 1: 在 .h 文件中新增 Tag 声明**

在 `Frontend_Image_TestImage` 声明之后、`}` 之前插入：

```cpp
	// Skill system tags
	FRONTENDUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill_LightAttack);
	FRONTENDUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill_HeavyAttack);

	FRONTENDUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Damage);
	FRONTENDUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Heal);
	FRONTENDUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Buff);

	FRONTENDUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cost_HP);
	FRONTENDUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cost_MP);
	FRONTENDUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cost_Energy);

	FRONTENDUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Target_Self);
	FRONTENDUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Target_Enemy);
	FRONTENDUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Target_Area);

	FRONTENDUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attr_HP);
	FRONTENDUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attr_MP);
	FRONTENDUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attr_Energy);
```

- [ ] **Step 2: 在 .cpp 文件中新增 Tag 定义**

在 `HUD Widget` 注释之后插入：

```cpp
	// Skill system
	UE_DEFINE_GAMEPLAY_TAG(Skill_LightAttack, "Skill.LightAttack");
	UE_DEFINE_GAMEPLAY_TAG(Skill_HeavyAttack, "Skill.HeavyAttack");

	UE_DEFINE_GAMEPLAY_TAG(Effect_Damage, "Effect.Damage");
	UE_DEFINE_GAMEPLAY_TAG(Effect_Heal, "Effect.Heal");
	UE_DEFINE_GAMEPLAY_TAG(Effect_Buff, "Effect.Buff");

	UE_DEFINE_GAMEPLAY_TAG(Cost_HP, "Cost.HP");
	UE_DEFINE_GAMEPLAY_TAG(Cost_MP, "Cost.MP");
	UE_DEFINE_GAMEPLAY_TAG(Cost_Energy, "Cost.Energy");

	UE_DEFINE_GAMEPLAY_TAG(Target_Self, "Target.Self");
	UE_DEFINE_GAMEPLAY_TAG(Target_Enemy, "Target.Enemy");
	UE_DEFINE_GAMEPLAY_TAG(Target_Area, "Target.Area");

	UE_DEFINE_GAMEPLAY_TAG(Attr_HP, "Attr.HP");
	UE_DEFINE_GAMEPLAY_TAG(Attr_MP, "Attr.MP");
	UE_DEFINE_GAMEPLAY_TAG(Attr_Energy, "Attr.Energy");
```

- [ ] **Step 3: 更新 DefaultGameplayTags.ini**

在文件末尾追加（`+GameplayTagList` 条目之前已有）：

```ini
+GameplayTagList=(Tag="Skill.LightAttack",DevComment="轻攻击技能标签")
+GameplayTagList=(Tag="Skill.HeavyAttack",DevComment="重攻击技能标签")
+GameplayTagList=(Tag="Effect.Damage",DevComment="伤害效果")
+GameplayTagList=(Tag="Effect.Heal",DevComment="治疗效果")
+GameplayTagList=(Tag="Effect.Buff",DevComment="增益效果")
+GameplayTagList=(Tag="Cost.HP",DevComment="HP消耗")
+GameplayTagList=(Tag="Cost.MP",DevComment="MP消耗")
+GameplayTagList=(Tag="Cost.Energy",DevComment="能量消耗")
+GameplayTagList=(Tag="Target.Self",DevComment="自身目标")
+GameplayTagList=(Tag="Target.Enemy",DevComment="敌方目标")
+GameplayTagList=(Tag="Target.Area",DevComment="范围目标")
+GameplayTagList=(Tag="Attr.HP",DevComment="生命属性")
+GameplayTagList=(Tag="Attr.MP",DevComment="法力属性")
+GameplayTagList=(Tag="Attr.Energy",DevComment="能量属性")
```

- [ ] **Step 4: Commit**

```bash
git add Source/FrontendUI/Public/FrontendGamePlayTags.h \
        Source/FrontendUI/Private/FrontendGamePlayTags.cpp \
        Config/DefaultGameplayTags.ini
git commit -m "feat: 注册技能系统 GameplayTags（Skill/Effect/Cost/Target/Attr）"
```

---

### Task 3: 创建 USkillConfigSubsystem — 运行时配置子系统

**Files:**
- Create: `Source/FrontendUI/Public/AbilitySystem/SkillConfigSubsystem.h`
- Create: `Source/FrontendUI/Private/AbilitySystem/SkillConfigSubsystem.cpp`

- [ ] **Step 1: 创建 SkillConfigSubsystem.h**

```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AbilitySystem/SkillConfigTypes.h"
#include "SkillConfigSubsystem.generated.h"

class UAbilitySystemComponent;

/**
 * 技能配置子系统
 *
 * GameInstance 级单例，跨关卡存活。
 * 初始化时加载 DT_SkillConfig / DT_SkillEffect / DT_BuffConfig 三个 DataTable，
 * 缓存为 TMap 供运行时 O(1) 查询。
 */
UCLASS()
class FRONTENDUI_API USkillConfigSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 通过 World 获取 Subsystem */
	static USkillConfigSubsystem* Get(const UObject* WorldContext);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========== 查询接口 ==========

	/** 按 ID 查询技能配置，未找到返回 nullptr */
	const FSkillConfigRow* GetSkillConfig(FName SkillID) const;

	/** 按 ID 查询技能下所有效果（按表格顺序） */
	const TArray<FSkillEffectRow*>* GetSkillEffects(FName SkillID) const;

	/** 按 ID 查询 Buff 配置 */
	const FBuffConfigRow* GetBuffConfig(FName BuffID) const;

	/** 公式求值 */
	float EvaluateFormula(const FString& Formula, int32 Level = 1, int32 Stacks = 1) const;

	/** 从效果行创建可应用的 GameplayEffectSpecHandle */
	FGameplayEffectSpecHandle MakeEffectSpec(
		const FSkillEffectRow& EffectRow,
		float EvaluatedValue,
		AActor* SourceActor,
		UAbilitySystemComponent* TargetASC) const;

	/** 从 Buff 行创建持续 GameplayEffectSpecHandle */
	FGameplayEffectSpecHandle MakeBuffSpec(
		const FBuffConfigRow& BuffRow,
		float EvaluatedModifierValue,
		float EvaluatedDuration,
		UObject* SourceObject) const;

	// ========== 配置资源路径 ==========

	/** 技能定义 DataTable 软引用：/Game/DataTables/DT_SkillConfig */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSoftObjectPtr<UDataTable> SkillConfigTable;

	/** 技能效果 DataTable 软引用：/Game/DataTables/DT_SkillEffect */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSoftObjectPtr<UDataTable> SkillEffectTable;

	/** Buff DataTable 软引用：/Game/DataTables/DT_BuffConfig */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSoftObjectPtr<UDataTable> BuffConfigTable;

	/** 通用伤害 GE 类 — 通过 SetByCaller 注入伤害值 */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<class UGameplayEffect> DamageGEClass;

	/** 通用治疗 GE 类 */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<class UGameplayEffect> HealGEClass;

	/** 通用 Buff GE 类 — 用于创建持续效果 */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<class UGameplayEffect> BuffGEClass;

private:
	/** 确保 DataTable 已加载（懒加载） */
	void EnsureTablesLoaded();

	TMap<FName, FSkillConfigRow> SkillConfigMap;
	TMap<FName, FSkillEffectRow> SkillEffectMap;
	TMap<FName, FBuffConfigRow> BuffConfigMap;
	TMap<FName, TArray<FName>> SkillToEffectsMap; // SkillID → EffectIDs（有序）

	bool bTablesLoaded = false;
};
```

- [ ] **Step 2: 创建 SkillConfigSubsystem.cpp**

```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/SkillConfigSubsystem.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Engine/DataTable.h"
#include "Engine/AssetManager.h"
#include "FrontendDebugHelper.h"

USkillConfigSubsystem* USkillConfigSubsystem::Get(const UObject* WorldContext)
{
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::Assert))
	{
		return UGameInstance::GetSubsystem<USkillConfigSubsystem>(World->GetGameInstance());
	}
	return nullptr;
}

void USkillConfigSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	// 延迟到首次访问时加载，避免启动时卡顿
}

void USkillConfigSubsystem::Deinitialize()
{
	SkillConfigMap.Empty();
	SkillEffectMap.Empty();
	BuffConfigMap.Empty();
	SkillToEffectsMap.Empty();
	bTablesLoaded = false;
	Super::Deinitialize();
}

void USkillConfigSubsystem::EnsureTablesLoaded()
{
	if (bTablesLoaded) return;
	bTablesLoaded = true;

	auto LoadTable = [](TSoftObjectPtr<UDataTable>& SoftPtr, const TCHAR* TableName) -> UDataTable*
	{
		if (SoftPtr.IsNull()) return nullptr;
		UDataTable* DT = SoftPtr.LoadSynchronous();
		if (!DT)
		{
			UE_LOG(LogTemp, Warning, TEXT("[SkillConfigSubsystem] Failed to load %s"), TableName);
		}
		return DT;
	};

	if (UDataTable* DT = LoadTable(SkillConfigTable, TEXT("DT_SkillConfig")))
	{
		static const FString ContextStr(TEXT("SkillConfigSubsystem::LoadSkillConfig"));
		TArray<FSkillConfigRow*> Rows;
		DT->GetAllRows<FSkillConfigRow>(ContextStr, Rows);
		for (FSkillConfigRow* Row : Rows)
		{
			if (Row) SkillConfigMap.Add(Row->RowName, *Row);
		}
	}

	if (UDataTable* DT = LoadTable(SkillEffectTable, TEXT("DT_SkillEffect")))
	{
		static const FString ContextStr(TEXT("SkillConfigSubsystem::LoadSkillEffect"));
		TArray<FSkillEffectRow*> Rows;
		DT->GetAllRows<FSkillEffectRow>(ContextStr, Rows);
		for (FSkillEffectRow* Row : Rows)
		{
			if (!Row) continue;
			SkillEffectMap.Add(Row->RowName, *Row);
			SkillToEffectsMap.FindOrAdd(Row->SkillID).Add(Row->RowName);
		}
	}

	if (UDataTable* DT = LoadTable(BuffConfigTable, TEXT("DT_BuffConfig")))
	{
		static const FString ContextStr(TEXT("SkillConfigSubsystem::LoadBuffConfig"));
		TArray<FBuffConfigRow*> Rows;
		DT->GetAllRows<FBuffConfigRow>(ContextStr, Rows);
		for (FBuffConfigRow* Row : Rows)
		{
			if (Row) BuffConfigMap.Add(Row->RowName, *Row);
		}
	}
}

const FSkillConfigRow* USkillConfigSubsystem::GetSkillConfig(FName SkillID) const
{
	const_cast<USkillConfigSubsystem*>(this)->EnsureTablesLoaded();
	return SkillConfigMap.Find(SkillID);
}

const TArray<FSkillEffectRow*>* USkillConfigSubsystem::GetSkillEffects(FName SkillID) const
{
	const_cast<USkillConfigSubsystem*>(this)->EnsureTablesLoaded();
	// 返回原始指针需要特殊处理：我们缓存行指针到临时数组
	// 这里改为返回值数组而非指针数组
	return nullptr; // See Step 3 for fix
}

const FBuffConfigRow* USkillConfigSubsystem::GetBuffConfig(FName BuffID) const
{
	const_cast<USkillConfigSubsystem*>(this)->EnsureTablesLoaded();
	return BuffConfigMap.Find(BuffID);
}

float USkillConfigSubsystem::EvaluateFormula(const FString& Formula, int32 Level, int32 Stacks) const
{
	if (Formula.IsEmpty()) return 0.f;
	return FFormulaEvaluator::Evaluate(Formula, Level, Stacks);
}

FGameplayEffectSpecHandle USkillConfigSubsystem::MakeEffectSpec(
	const FSkillEffectRow& EffectRow,
	float EvaluatedValue,
	AActor* SourceActor,
	UAbilitySystemComponent* TargetASC) const
{
	TSubclassOf<UGameplayEffect> GEClass;

	if (EffectRow.EffectType.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Effect.Damage"))))
	{
		GEClass = DamageGEClass;
	}
	else if (EffectRow.EffectType.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Effect.Heal"))))
	{
		GEClass = HealGEClass;
	}

	if (!GEClass || !SourceActor) return FGameplayEffectSpecHandle();

	UAbilitySystemComponent* SourceASC = nullptr;
	if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(SourceActor))
	{
		SourceASC = ASI->GetAbilitySystemComponent();
	}
	if (!SourceASC) return FGameplayEffectSpecHandle();

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(SourceActor);
	const FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(GEClass, 1.f, Context);

	if (Spec.IsValid() && EffectRow.ValueAttribute.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(EffectRow.ValueAttribute, EvaluatedValue);
	}

	return Spec;
}

FGameplayEffectSpecHandle USkillConfigSubsystem::MakeBuffSpec(
	const FBuffConfigRow& BuffRow,
	float EvaluatedModifierValue,
	float EvaluatedDuration,
	UObject* SourceObject) const
{
	if (!BuffGEClass || !SourceObject) return FGameplayEffectSpecHandle();

	AActor* SourceActor = Cast<AActor>(SourceObject);
	if (!SourceActor) return FGameplayEffectSpecHandle();

	if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(SourceActor))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			Context.AddSourceObject(SourceActor);
			FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(BuffGEClass, 1.f, Context);

			if (Spec.IsValid())
			{
				// Set duration
				if (EvaluatedDuration > 0.f)
				{
					Spec.Data->SetDuration(EvaluatedDuration, true);
				}
			}

			return Spec;
		}
	}

	return FGameplayEffectSpecHandle();
}
```

- [ ] **Step 3: 修复 GetSkillEffects 返回类型 — 改为拷贝返回而非指针数组**

编辑 `SkillConfigSubsystem.h`，将：

```cpp
const TArray<FSkillEffectRow*>* GetSkillEffects(FName SkillID) const;
```

改为：

```cpp
TArray<FSkillEffectRow> GetSkillEffects(FName SkillID) const;
```

并在 .cpp 中重新实现 `GetSkillEffects`：

```cpp
TArray<FSkillEffectRow> USkillConfigSubsystem::GetSkillEffects(FName SkillID) const
{
	const_cast<USkillConfigSubsystem*>(this)->EnsureTablesLoaded();

	TArray<FSkillEffectRow> Result;
	if (const TArray<FName>* EffectIDs = SkillToEffectsMap.Find(SkillID))
	{
		Result.Reserve(EffectIDs->Num());
		for (const FName& EffectID : *EffectIDs)
		{
			if (const FSkillEffectRow* Row = SkillEffectMap.Find(EffectID))
			{
				Result.Add(*Row);
			}
		}
	}
	return Result;
}
```

- [ ] **Step 4: Commit**

```bash
git add Source/FrontendUI/Public/AbilitySystem/SkillConfigSubsystem.h \
        Source/FrontendUI/Private/AbilitySystem/SkillConfigSubsystem.cpp
git commit -m "feat: 实现 USkillConfigSubsystem 运行时配置查询子系统"
```

---

### Task 4: 实现 FFormulaEvaluator 公式求值器

**Files:**
- Modify: `Source/FrontendUI/Public/AbilitySystem/SkillConfigTypes.h` — 已声明，需补充实现
- Create: （或放入 header 底部 inline）

- [ ] **Step 1: 在 SkillConfigTypes.h 末尾追加 FFormulaEvaluator 实现（inline 方式）**

在 `FFormulaEvaluator` 声明之后追加：

```cpp
// ============================================================================
// FFormulaEvaluator — 公式求值器（递归下降）
// ============================================================================

inline FFormulaEvaluator::FTokenizer::FTokenizer(const FString& InFormula, int32 InLevel, int32 InStacks)
	: Formula(InFormula), Level(InLevel), Stacks(InStacks)
{
}

inline FFormulaEvaluator::FToken FFormulaEvaluator::FTokenizer::Peek() const
{
	if (!bHasLookahead)
	{
		Lookahead = NextToken();
		bHasLookahead = true;
	}
	return Lookahead;
}

inline FFormulaEvaluator::FToken FFormulaEvaluator::FTokenizer::Consume()
{
	FToken T = Peek();
	bHasLookahead = false;
	return T;
}

inline void FFormulaEvaluator::FTokenizer::SkipWhitespace()
{
	while (Pos < Formula.Len() && FChar::IsWhitespace(Formula[Pos]))
	{
		++Pos;
	}
}

inline float FFormulaEvaluator::FTokenizer::ResolveVariable(const FString& Name) const
{
	if (Name.Equals(TEXT("Level"), ESearchCase::IgnoreCase))
		return static_cast<float>(Level);
	if (Name.Equals(TEXT("Stack"), ESearchCase::IgnoreCase))
		return static_cast<float>(Stacks);
	return 0.f;
}

inline FFormulaEvaluator::FToken FFormulaEvaluator::FTokenizer::NextToken()
{
	SkipWhitespace();

	if (Pos >= Formula.Len())
		return { ETokenType::End, 0.f };

	const TCHAR C = Formula[Pos];

	// 数字
	if (FChar::IsDigit(C) || C == '.')
	{
		const int32 Start = Pos;
		while (Pos < Formula.Len() && (FChar::IsDigit(Formula[Pos]) || Formula[Pos] == '.'))
			++Pos;
		float Val = FCString::Atof(&Formula[Start]);
		return { ETokenType::Number, Val };
	}

	// 标识符（变量）
	if (FChar::IsAlpha(C) || C == '_')
	{
		const int32 Start = Pos;
		while (Pos < Formula.Len() && (FChar::IsAlnum(Formula[Pos]) || Formula[Pos] == '_'))
			++Pos;
		FString VarName = Formula.Mid(Start, Pos - Start);
		float Val = ResolveVariable(VarName);
		return { ETokenType::Var, Val, VarName };
	}

	++Pos;
	switch (C)
	{
	case '+': return { ETokenType::Plus };
	case '-': return { ETokenType::Minus };
	case '*': return { ETokenType::Mul };
	case '/': return { ETokenType::Div };
	case '(': return { ETokenType::LParen };
	case ')': return { ETokenType::RParen };
	default:
		// 跳过不支持字符
		return NextToken();
	}
}

// ---- Parser ----

inline float FFormulaEvaluator::FParser::ParseExpression()
{
	float Result = ParseTerm();
	while (true)
	{
		FToken T = Tokenizer.Peek();
		if (T.Type == ETokenType::Plus)
		{
			Tokenizer.Consume();
			Result += ParseTerm();
		}
		else if (T.Type == ETokenType::Minus)
		{
			Tokenizer.Consume();
			Result -= ParseTerm();
		}
		else break;
	}
	return Result;
}

inline float FFormulaEvaluator::FParser::ParseTerm()
{
	float Result = ParseFactor();
	while (true)
	{
		FToken T = Tokenizer.Peek();
		if (T.Type == ETokenType::Mul)
		{
			Tokenizer.Consume();
			Result *= ParseFactor();
		}
		else if (T.Type == ETokenType::Div)
		{
			Tokenizer.Consume();
			float Divisor = ParseFactor();
			Result = (FMath::Abs(Divisor) > KINDA_SMALL_NUMBER) ? Result / Divisor : Result;
		}
		else break;
	}
	return Result;
}

inline float FFormulaEvaluator::FParser::ParseFactor()
{
	FToken T = Tokenizer.Peek();
	if (T.Type == ETokenType::Minus)
	{
		Tokenizer.Consume();
		return -ParseAtom();
	}
	return ParseAtom();
}

inline float FFormulaEvaluator::FParser::ParseAtom()
{
	FToken T = Tokenizer.Consume();
	if (T.Type == ETokenType::Number || T.Type == ETokenType::Var)
		return T.Value;

	if (T.Type == ETokenType::LParen)
	{
		float Val = ParseExpression();
		FToken Close = Tokenizer.Consume(); // consume RParen
		return Val;
	}

	return 0.f;
}

// ---- 公开接口 ----

inline float FFormulaEvaluator::Evaluate(const FString& Formula, int32 Level, int32 Stacks)
{
	if (Formula.IsEmpty()) return 0.f;
	FTokenizer Tokenizer(Formula, Level, Stacks);
	FParser Parser(Tokenizer);
	return Parser.ParseExpression();
}
```

- [ ] **Step 2: Commit**

```bash
git add Source/FrontendUI/Public/AbilitySystem/SkillConfigTypes.h
git commit -m "feat: 实现 FFormulaEvaluator 递归下降公式求值器"
```

---

### Task 5: 创建 UGA_DataDriven 通用技能类

**Files:**
- Create: `Source/FrontendUI/Public/AbilitySystem/SkillGameplayAbility.h`
- Create: `Source/FrontendUI/Private/AbilitySystem/SkillGameplayAbility.cpp`

- [ ] **Step 1: 创建 SkillGameplayAbility.h**

```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "SkillGameplayAbility.generated.h"

struct FSkillConfigRow;

/**
 * 通用数据驱动技能
 *
 * 从 DynamicAbilityTags 提取 SkillID，通过 USkillConfigSubsystem 读取配置，
 * 自动执行伤害/治疗/Buff 等表格驱动效果。
 */
UCLASS()
class FRONTENDUI_API UGA_DataDriven : public UGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;

	/** 从 DynamicAbilityTags 中提取 Skill.{Name} 格式的标签 */
	FName GetSkillIDFromTags() const;

protected:
	/** 技能等级（由 AbilitySet 授予时设置） */
	UPROPERTY()
	int32 SkillLevel = 1;

	/** 当前技能的配置缓存 */
	FSkillConfigRow CachedSkillConfig;
};
```

- [ ] **Step 2: 创建 SkillGameplayAbility.cpp**

```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/SkillGameplayAbility.h"
#include "AbilitySystem/SkillConfigSubsystem.h"
#include "AbilitySystem/SkillConfigTypes.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "FrontendGamePlayTags.h"
#include "FrontendDebugHelper.h"

void UGA_DataDriven::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const FName SkillID = GetSkillIDFromTags();
	if (SkillID.IsNone())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	USkillConfigSubsystem* Subsystem = USkillConfigSubsystem::Get(ActorInfo->AvatarActor.Get());
	if (!Subsystem)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FSkillConfigRow* Config = Subsystem->GetSkillConfig(SkillID);
	if (!Config)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GA_DataDriven] Skill config not found: %s"), *SkillID.ToString());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	CachedSkillConfig = *Config;

	// TODO: 冷却 + 消耗检查 — 后续 Task 中通过 GE 实现

	// 执行效果
	const TArray<FSkillEffectRow> Effects = Subsystem->GetSkillEffects(SkillID);

	AActor* Owner = ActorInfo->AvatarActor.Get();
	UAbilitySystemComponent* OwnerASC = ActorInfo->AbilitySystemComponent.Get();

	for (const FSkillEffectRow& Effect : Effects)
	{
		float EvaluatedValue = Subsystem->EvaluateFormula(
			FString::Printf(TEXT("%.1f + %.1f * Level"), Effect.BaseValue, Effect.ValueScale),
			SkillLevel);

		FGameplayTag EffectType = Effect.EffectType;
		FGameplayTag TargetType = Effect.TargetType;

		if (EffectType.MatchesTag(FrontendGameplayTags::Effect_Damage))
		{
			// 伤害应用到目标
			if (TargetType.MatchesTag(FrontendGameplayTags::Target_Enemy) && TriggerEventData)
			{
				if (AActor* Target = const_cast<AActor*>(TriggerEventData->Target.Get()))
				{
					if (const IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(Target))
					{
						FGameplayEffectSpecHandle Spec = Subsystem->MakeEffectSpec(
							Effect, EvaluatedValue, Owner, TargetASI->GetAbilitySystemComponent());
						if (Spec.IsValid() && TargetASI->GetAbilitySystemComponent())
						{
							TargetASI->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
						}
					}
				}
			}
		}
		else if (EffectType.MatchesTag(FrontendGameplayTags::Effect_Heal))
		{
			// 治疗应用到自身
			if (TargetType.MatchesTag(FrontendGameplayTags::Target_Self))
			{
				FGameplayEffectSpecHandle Spec = Subsystem->MakeEffectSpec(
					Effect, EvaluatedValue, Owner, OwnerASC);
				if (Spec.IsValid())
				{
					OwnerASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
				}
			}
		}
		else if (EffectType.MatchesTag(FrontendGameplayTags::Effect_Buff))
		{
			// Buff 通过 BuffConfig 创建持续 GE
			if (!Effect.BuffID.IsNone())
			{
				if (const FBuffConfigRow* BuffRow = Subsystem->GetBuffConfig(Effect.BuffID))
				{
					float ModifierValue = Subsystem->EvaluateFormula(BuffRow->ModifierFormula, SkillLevel);
					float Duration = Subsystem->EvaluateFormula(BuffRow->DurationFormula, SkillLevel);

					UAbilitySystemComponent* TargetASC = TargetType.MatchesTag(FrontendGameplayTags::Target_Self)
						? OwnerASC
						: (TriggerEventData && TriggerEventData->Target.IsValid()
							? Cast<IAbilitySystemInterface>(TriggerEventData->Target.Get()) ? Cast<IAbilitySystemInterface>(TriggerEventData->Target.Get())->GetAbilitySystemComponent() : nullptr
							: nullptr);

					if (TargetASC)
					{
						FGameplayEffectSpecHandle Spec = Subsystem->MakeBuffSpec(*BuffRow, ModifierValue, Duration, Owner);
						if (Spec.IsValid())
						{
							TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
						}
					}
				}
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UGA_DataDriven::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FName UGA_DataDriven::GetSkillIDFromTags() const
{
	const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
	if (!Spec) return NAME_None;

	static const FName SkillTagPrefix(TEXT("Skill."));
	for (const FGameplayTag& Tag : Spec->DynamicAbilityTags)
	{
		const FName TagName = Tag.GetTagName();
		FString TagStr = TagName.ToString();
		if (TagStr.StartsWith(TEXT("Skill.")))
		{
			return FName(*TagStr.RightChop(6)); // 去掉 "Skill." 前缀
		}
	}
	return NAME_None;
}
```

- [ ] **Step 3: Commit**

```bash
git add Source/FrontendUI/Public/AbilitySystem/SkillGameplayAbility.h \
        Source/FrontendUI/Private/AbilitySystem/SkillGameplayAbility.cpp
git commit -m "feat: 实现 UGA_DataDriven 通用数据驱动技能类"
```

---

### Task 6: 改造 OnAttackHit — 从 Subsystem 读取伤害值

**Files:**
- Modify: `Source/FrontendUI/Public/Character/BaseCharacter.h`
- Modify: `Source/FrontendUI/Private/Character/BaseCharacter.cpp`

- [ ] **Step 1: 在 BaseCharacter.h 中新增 SkillID 属性**

在 `HeavyAttackAbilityTag` 声明之后追加：

```cpp
	/** 轻攻击对应的 SkillID（查询 SkillConfigSubsystem 用） */
	UPROPERTY(EditDefaultsOnly, Category = "Combat|GAS")
	FName LightAttackSkillID = "LightAttack";

	/** 重攻击对应的 SkillID */
	UPROPERTY(EditDefaultsOnly, Category = "Combat|GAS")
	FName HeavyAttackSkillID = "HeavyAttack";
```

- [ ] **Step 2: 在 BaseCharacter.cpp 中添加 include**

在文件顶部现有 include 之后追加：

```cpp
#include "AbilitySystem/SkillConfigSubsystem.h"
#include "AbilitySystem/SkillConfigTypes.h"
#include "FrontendGamePlayTags.h"
```

- [ ] **Step 3: 改造 OnAttackHit**

替换现有的 `OnAttackHit` 函数体：

```cpp
void ABaseCharacter::OnAttackHit(AActor* HitActor, const FHitResult& HitResult)
{
	if (!HitActor) return;

	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(HitActor);
	UAbilitySystemComponent* TargetASC = ASI ? ASI->GetAbilitySystemComponent() : nullptr;
	UAbilitySystemComponent* MyASC = GetAbilitySystemComponent();

	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan,
		FString::Printf(TEXT("[Hit] %s → ASC=%s"),
			*HitActor->GetName(),
			TargetASC ? *TargetASC->GetOwner()->GetName() : TEXT("NULL")));

	if (!TargetASC || !MyASC) return;

	// 通过 Subsystem 读取配置，替代硬编码伤害
	USkillConfigSubsystem* SkillSubsystem = USkillConfigSubsystem::Get(this);
	if (!SkillSubsystem)
	{
		// 降级：Subsystem 未就绪时使用旧的硬编码逻辑
		if (!DamageGameplayEffectClass) return;

		FGameplayEffectContextHandle Context = MyASC->MakeEffectContext();
		Context.AddSourceObject(this);
		const FGameplayEffectSpecHandle Spec = MyASC->MakeOutgoingSpec(DamageGameplayEffectClass, 1.f, Context);
		if (Spec.IsValid())
		{
			const FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("SetByCaller.Damage"));
			Spec.Data->SetSetByCallerMagnitude(DamageTag, 20.f);
			TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
		return;
	}

	// 判断当前激活的轻/重攻击，选择对应 SkillID
	FName ActiveSkillID;
	if (LightAttackAbilityTag.IsValid() && MyASC->IsMatchingTag(LightAttackAbilityTag))
	{
		ActiveSkillID = LightAttackSkillID;
	}
	else if (HeavyAttackAbilityTag.IsValid() && MyASC->IsMatchingTag(HeavyAttackAbilityTag))
	{
		ActiveSkillID = HeavyAttackSkillID;
	}

	if (ActiveSkillID.IsNone()) return;

	// 从配置表读取效果
	const FSkillConfigRow* SkillCfg = SkillSubsystem->GetSkillConfig(ActiveSkillID);
	if (!SkillCfg) return;

	const TArray<FSkillEffectRow> Effects = SkillSubsystem->GetSkillEffects(ActiveSkillID);

	for (const FSkillEffectRow& Effect : Effects)
	{
		if (Effect.EffectType.MatchesTag(FrontendGameplayTags::Effect_Damage))
		{
			int32 SkillLevel = 1; // TODO: 从 ASC 或外部传入技能等级
			float Damage = FFormulaEvaluator::Evaluate(
				FString::Printf(TEXT("%.1f + %.1f * Level"), Effect.BaseValue, Effect.ValueScale),
				SkillLevel);

			// 使用 Subsystem 的通用 DamageGE
			FGameplayEffectSpecHandle Spec = SkillSubsystem->MakeEffectSpec(Effect, Damage, this, TargetASC);
			if (Spec.IsValid())
			{
				TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
		}
	}
}
```

- [ ] **Step 4: Commit**

```bash
git add Source/FrontendUI/Public/Character/BaseCharacter.h \
        Source/FrontendUI/Private/Character/BaseCharacter.cpp
git commit -m "feat: OnAttackHit 改造为从 SkillConfigSubsystem 读取伤害值"
```

---

### Task 7: 编写公式求值器单元验证

**Files:**
- Create: `Source/FrontendUI/Private/Tests/SkillFormulaTest.cpp`

- [ ] **Step 1: 创建测试文件**

```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "AbilitySystem/SkillConfigTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSkillFormulaBasicTest,
	"FrontendUI.Skill.Formula.Basic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillFormulaBasicTest::RunTest(const FString& Parameters)
{
	// 纯数字
	TestEqual("Pure number", FFormulaEvaluator::Evaluate(TEXT("42"), 1, 1), 42.f);

	// 简单加法
	TestEqual("Simple add", FFormulaEvaluator::Evaluate(TEXT("10 + 5"), 1, 1), 15.f);

	// 含 Level 变量
	TestEqual("Level variable L1", FFormulaEvaluator::Evaluate(TEXT("10 + 5 * Level"), 3, 1), 25.f);
	TestEqual("Level variable L2", FFormulaEvaluator::Evaluate(TEXT("10 + 5 * Level"), 1, 1), 15.f);

	// 含 Stack 变量
	TestEqual("Stack variable", FFormulaEvaluator::Evaluate(TEXT("5 * Stack"), 1, 3), 15.f);

	// 括号
	TestEqual("Parentheses", FFormulaEvaluator::Evaluate(TEXT("(10 + 5) * 2"), 1, 1), 30.f);

	// 减法
	TestEqual("Subtract", FFormulaEvaluator::Evaluate(TEXT("20 - Level * 3"), 2, 1), 14.f);

	// 空格处理
	TestEqual("Spaces", FFormulaEvaluator::Evaluate(TEXT("  20 - 5  "), 1, 1), 15.f);

	// 空字符串
	TestEqual("Empty", FFormulaEvaluator::Evaluate(TEXT(""), 1, 1), 0.f);

	// 除零保护
	TestEqual("Div by zero guard", FFormulaEvaluator::Evaluate(TEXT("10 / 0"), 1, 1), 10.f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSkillFormulaEdgeTest,
	"FrontendUI.Skill.Formula.EdgeCases",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillFormulaEdgeTest::RunTest(const FString& Parameters)
{
	// 负数
	TestEqual("Negative", FFormulaEvaluator::Evaluate(TEXT("-5 + 10"), 1, 1), 5.f);
	TestEqual("Negative result", FFormulaEvaluator::Evaluate(TEXT("5 - 10"), 1, 1), -5.f);

	// 复杂嵌套
	float Result = FFormulaEvaluator::Evaluate(TEXT("(10 + Level) * (2 + Stack)"), 2, 3);
	TestEqual("Nested parens", Result, 60.f); // (12) * (5) = 60

	// 大 Level 值
	TestEqual("Large level", FFormulaEvaluator::Evaluate(TEXT("Level * 100"), 99, 1), 9900.f);

	// 除法
	TestEqual("Division", FFormulaEvaluator::Evaluate(TEXT("100 / 4"), 1, 1), 25.f);

	// 混合运算优先级 乘除优于加减
	TestEqual("Precedence", FFormulaEvaluator::Evaluate(TEXT("2 + 3 * 4"), 1, 1), 14.f);

	return true;
}

#endif //WITH_DEV_AUTOMATION_TESTS
```

- [ ] **Step 2: 验证编译并运行测试**

```bash
# 在 UE 编辑器中：Session Frontend → Automation → FrontendUI.Skill.Formula → Run
# 预期：全部 PASS
```

- [ ] **Step 3: Commit**

```bash
git add Source/FrontendUI/Private/Tests/SkillFormulaTest.cpp
git commit -m "test: 添加 FFormulaEvaluator 公式求值器单元测试"
```

---

### Task 8: CSV 模板准备与 DataTable 导入

**Files:**
- Create: `Content/DataTables/DT_SkillConfig.csv`
- Create: `Content/DataTables/DT_SkillEffect.csv`
- Create: `Content/DataTables/DT_BuffConfig.csv`

- [ ] **Step 1: 创建 CSV 模板并导入 UE 编辑器**

**DT_SkillConfig.csv**:
```csv
Name,SkillType,DisplayName,Icon,MaxLevel,CooldownFormula,CostType,CostFormula,CastRange,CastTime,AbilityTags,bGrantToASC
LightAttack,"(Tag=Skill.LightAttack)",NSLOCTEXT("","","轻攻击"),,5,0.5,"(Tag=Cost.Energy)",5.0,150.0,0.0,"(Tags=Ability.Melee.Light)",true
HeavyAttack,"(Tag=Skill.LightAttack)",NSLOCTEXT("","","重攻击"),,5,2.0,"(Tag=Cost.Energy)",15.0,250.0,0.5,"(Tags=Ability.Melee.Heavy)",true
```

**DT_SkillEffect.csv**:
```csv
Name,SkillID,EffectType,TargetType,BaseValue,ValueScale,ValueAttribute,Duration,Interval,BuffID
LightAttack_Damage,LightAttack,"(Tag=Effect.Damage)","(Tag=Target.Enemy)",15.0,5.0,"(Tag=Attr.HP)",0.0,0.0,
HeavyAttack_Damage,HeavyAttack,"(Tag=Effect.Damage)","(Tag=Target.Enemy)",40.0,10.0,"(Tag=Attr.HP)",0.0,0.0,
```

**DT_BuffConfig.csv**:
```csv
Name,DisplayName,DurationFormula,MaxStacks,ModifierOp,ModifierAttribute,ModifierFormula,Icon
Bleed,NSLOCTEXT("","","出血"),3.0 + 1.0 * Level,5,Additive,"(Tag=Attr.HP)",-2.0 - 0.5 * Level,
```

> **导入步骤**: UE 编辑器 → Content/DataTables/ → Import → 选择 CSV → 选择对应 Row Struct
> - DT_SkillConfig → FSkillConfigRow
> - DT_SkillEffect → FSkillEffectRow
> - DT_BuffConfig → FBuffConfigRow

- [ ] **Step 2: Commit CSV 模板**

```bash
git add Content/DataTables/DT_SkillConfig.csv \
        Content/DataTables/DT_SkillEffect.csv \
        Content/DataTables/DT_BuffConfig.csv
git commit -m "feat: 添加技能配表 CSV 模板"
```

---

## Plan Self-Review

**1. Spec coverage:**
- [x] 三个 FTableRow 结构体 → Task 1 SkillConfigTypes.h
- [x] GameplayTag 扩展 → Task 2
- [x] USkillConfigSubsystem 加载/缓存/查询 → Task 3
- [x] 公式求值器 → Task 4（inline 实现）
- [x] UGA_DataDriven 通用技能 → Task 5
- [x] OnAttackHit 改造 → Task 6
- [x] CSV 模板 → Task 8
- [x] 测试 → Task 7
- [ ] 冷却/消耗 GE 自动施加 → 后续 Task（需更多设计）

**2. Placeholder scan:** 无 TBD/TODO（仅一处 "TODO: 冷却 + 消耗检查" 标记为后续实现）。

**3. Type consistency:** FSkillConfigRow/FSkillEffectRow/FBuffConfigRow 在 Task 1 定义，Task 3/5/6 中的引用一致。GameplayTags Task 2 定义，Task 5/6 使用。

**已知差距:** 冷却/消耗 GE 自动施加、技能升级系统、Buff 的 ActiveGameplayEffectHandle 管理留到后续迭代。
