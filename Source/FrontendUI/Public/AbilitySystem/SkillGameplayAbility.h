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

	virtual bool CheckCost(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ApplyCost(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;

	virtual bool CheckCooldown(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ApplyCooldown(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;

	/** 从 DynamicAbilityTags 中提取 Skill.{Name} 格式的标签 */
	FName GetSkillIDFromTags() const;

protected:
	/** 技能等级（由 AbilitySet 授予时设置） */
	UPROPERTY()
	int32 SkillLevel = 1;

	/** 当前技能的配置缓存 */
	UPROPERTY()
	FSkillConfigRow CachedSkillConfig;

private:
	/** 遍历 DT_SkillEffect 中该技能的所有效果，按 TargetType 分派 GE */
	void ApplySkillEffects(const FGameplayAbilityActorInfo* ActorInfo);

	/** 向前方球形扫描，返回距离最近的非自身 ABaseCharacter */
	AActor* FindNearestEnemy(const FGameplayAbilityActorInfo* ActorInfo, float Range) const;

	/** 向前方球形扫描，返回范围内所有非自身 ABaseCharacter */
	TArray<AActor*> GetEnemiesInRange(const FGameplayAbilityActorInfo* ActorInfo, float Range) const;

	/** 对单个目标创建并应用 GE */
	void ApplyEffectToTarget(
		const FSkillEffectRow& EffectRow,
		float Value,
		const FGameplayAbilityActorInfo* ActorInfo,
		AActor* Target) const;
};

/**
 * 数据驱动被动技能类
 *
 * 授予时自动激活，按 DT_SkillEffect 配置应用效果：
 *  - TriggerTag 为空   → 立即应用 Infinite GE（永久属性加成）
 *  - TriggerTag 有效   → 监听对应 GameplayEvent，事件触发时应用瞬时/持续效果
 *
 * EndAbility 时自动移除所有已应用的 Infinite GE。
 */
UCLASS()
class FRONTENDUI_API UGA_Passive : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Passive();

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

private:
	/** 对单条效果行应用 Infinite GE，句柄存入 AppliedInfiniteEffects */
	void ApplyInfiniteEffect(const FSkillEffectRow& EffectRow, UAbilitySystemComponent* ASC, int32 Level);

	/**
	 * 将效果行按 TriggerTag 分组，并为每个唯一 Tag 注册一个 WaitGameplayEvent Task
	 * 多条相同 Tag 的效果共用一个 Task，事件触发时统一处理
	 */
	void RegisterTriggerEffect(const FSkillEffectRow& EffectRow, UAbilitySystemComponent* ASC, int32 Level);

	/** WaitGameplayEvent 回调，根据 EventTag 查表应用对应效果组 */
	UFUNCTION()
	void OnTriggerEventReceived(FGameplayEventData EventData);

	/** 已应用的 Infinite GE 句柄，EndAbility 时全部移除 */
	TArray<FActiveGameplayEffectHandle> AppliedInfiniteEffects;

	/** 按 TriggerTag 分组的效果列表，供 OnTriggerEventReceived 查表 */
	TMap<FGameplayTag, TArray<FSkillEffectRow>> TriggerEffectMap;

	/** 激活时的技能等级缓存，供触发回调使用 */
	int32 CachedLevel = 1;
};
