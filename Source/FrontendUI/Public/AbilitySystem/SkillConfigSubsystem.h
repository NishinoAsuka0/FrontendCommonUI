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
 * 初始化时从 UFrontendDeveloperSettings 读取 DataTable 路径，
 * 懒加载后缓存为 TMap 供运行时 O(1) 查询。
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
	const FSkillConfigRow* GetSkillConfig(FName SkillID);

	/** 按 ID 查询技能下所有效果（按表格顺序） */
	TArray<FSkillEffectRow> GetSkillEffects(FName SkillID);

	/** 按 ID 查询 Buff 配置 */
	const FBuffConfigRow* GetBuffConfig(FName BuffID);

	/** 公式求值 */
	float EvaluateFormula(const FString& Formula, int32 Level = 1, int32 Stacks = 1) const;

	/**
	 * 查询技能剩余冷却时间（秒）
	 * 返回 0 表示不在冷却中；UI 可每帧或定时轮询此函数
	 */
	UFUNCTION(BlueprintCallable, Category = "SkillSystem")
	float GetSkillCooldownTimeRemaining(FName SkillID, UAbilitySystemComponent* ASC) const;

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

	/**
	 * 为被动效果创建 Infinite GameplayEffectSpecHandle
	 * 返回的 Spec 可直接 ApplyGameplayEffectSpecToSelf，句柄需由调用方保存以便移除
	 */
	FGameplayEffectSpecHandle MakePassiveEffectSpec(
		const FSkillEffectRow& EffectRow,
		float EvaluatedValue,
		UAbilitySystemComponent* OwnerASC) const;

private:
	/** 确保 DataTable 已加载（懒加载，从 UFrontendDeveloperSettings 读取路径） */
	void EnsureTablesLoaded();

	TMap<FName, FSkillConfigRow> SkillConfigMap;
	TMap<FName, FSkillEffectRow> SkillEffectMap;
	TMap<FName, FBuffConfigRow> BuffConfigMap;
	TMap<FName, TArray<FName>> SkillToEffectsMap;
	bool bTablesLoaded = false;
};
