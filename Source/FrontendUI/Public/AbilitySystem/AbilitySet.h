// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AbilitySet.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;
class UGameplayEffect;
class UAttributeSet;

/**
 * AbilitySet 中单个 Ability 的授予条目。
 *
 * InputTag 会在授予时写入 FGameplayAbilitySpec::DynamicAbilityTags，
 * 为后续接入 Lyra 风格的 ASC::AbilityInputTagPressed() 预留。
 */
USTRUCT(BlueprintType)
struct FAbilitySet_GameplayAbility
{
	GENERATED_BODY()

	/** 授予的 Ability 类 */
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TSubclassOf<UGameplayAbility> Ability;

	/** 授予等级（默认 1，部分 Ability 支持按等级缩放伤害/时长） */
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	int32 AbilityLevel = 1;

	/** 输入 Tag：存储在 DynamicAbilityTags 中，供输入系统匹配 */
	UPROPERTY(EditDefaultsOnly, Category = "Ability", meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

/**
 * AbilitySet 中单个 GameplayEffect 的授予条目。
 */
USTRUCT(BlueprintType)
struct FAbilitySet_GameplayEffect
{
	GENERATED_BODY()

	/** 授予时立即应用到自身的 GE（如初始属性初始化） */
	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> GameplayEffect;

	/** GE 等级 */
	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	float EffectLevel = 1.0f;
};

/**
 * AbilitySet 中单个 AttributeSet 的授予条目。
 */
USTRUCT(BlueprintType)
struct FAbilitySet_AttributeSet
{
	GENERATED_BODY()

	/** 要创建的 AttributeSet 类 */
	UPROPERTY(EditDefaultsOnly, Category = "Attribute")
	TSubclassOf<UAttributeSet> AttributeSet;
};

/**
 * 一次性授予 Ability / GameplayEffect / AttributeSet 的 DataAsset。
 *
 * 用法：
 *   1. 在编辑器中创建 UAbilitySet DataAsset，填充 GrantedAbilities / GrantedEffects / GrantedAttributes
 *   2. 在角色或 Pawn 的 Details 面板中将此 Asset 加入 AbilitySets 数组
 *   3. ASC 初始化后调用 GiveToAbilitySystem(ASC, SourceObject)
 */
UCLASS(BlueprintType, Const)
class FRONTENDUI_API UAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 将本 AbilitySet 中的所有条目一次性授予目标 ASC */
	void GiveToAbilitySystem(UAbilitySystemComponent* ASC, UObject* SourceObject = nullptr) const;

	/** 要授予的 Ability 列表 */
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TArray<FAbilitySet_GameplayAbility> GrantedAbilities;

	/** 要应用的 GameplayEffect 列表（立即 ApplyToSelf） */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TArray<FAbilitySet_GameplayEffect> GrantedEffects;

	/** 要创建的 AttributeSet 列表 */
	UPROPERTY(EditDefaultsOnly, Category = "Attributes")
	TArray<FAbilitySet_AttributeSet> GrantedAttributes;
};
