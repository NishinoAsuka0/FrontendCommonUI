// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/SkillConfigTypes.h"
#include "SkillGameplayAbility.generated.h"

/**
 * 通用数据驱动技能类
 *
 * 从 DynamicAbilityTags 中提取 SkillID (格式: Skill.Xxx)，
 * 通过 USkillConfigSubsystem 查询配置与效果，按类型分派执行。
 *
 * 效果类型：
 *  - Effect.Damage → 对 Target.Enemy 造成伤害
 *  - Effect.Heal   → 对 Target.Self 恢复生命
 *  - Effect.Buff   → 对目标施加 Buff/Debuff
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
	UPROPERTY()
	FSkillConfigRow CachedSkillConfig;
};
