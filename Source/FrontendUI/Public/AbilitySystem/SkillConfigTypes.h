// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
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
