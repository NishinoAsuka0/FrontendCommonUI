// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
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

	/** 攻击蒙太奇（软引用，激活时同步加载并播放） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> AttackMontage;

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
		FToken Peek();
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

// ============================================================================
// FFormulaEvaluator — 公式求值器（递归下降）
// ============================================================================

inline FFormulaEvaluator::FTokenizer::FTokenizer(const FString& InFormula, int32 InLevel, int32 InStacks)
	: Formula(InFormula), Level(InLevel), Stacks(InStacks)
{
}

inline FFormulaEvaluator::FToken FFormulaEvaluator::FTokenizer::Peek()
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

	if (FChar::IsDigit(C) || C == '.')
	{
		const int32 Start = Pos;
		while (Pos < Formula.Len() && (FChar::IsDigit(Formula[Pos]) || Formula[Pos] == '.'))
			++Pos;
		float Val = FCString::Atof(&Formula[Start]);
		return { ETokenType::Number, Val };
	}

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
		Tokenizer.Consume(); // consume RParen
		return Val;
	}

	return 0.f;
}

// ---- Public API ----

inline float FFormulaEvaluator::Evaluate(const FString& Formula, int32 Level, int32 Stacks)
{
	if (Formula.IsEmpty()) return 0.f;
	FTokenizer Tokenizer(Formula, Level, Stacks);
	FParser Parser(Tokenizer);
	return Parser.ParseExpression();
}
