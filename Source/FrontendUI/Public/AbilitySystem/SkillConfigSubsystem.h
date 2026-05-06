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
	TArray<FSkillEffectRow> GetSkillEffects(FName SkillID) const;

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

	/** 技能定义 DataTable 软引用 */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSoftObjectPtr<UDataTable> SkillConfigTable;

	/** 技能效果 DataTable 软引用 */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSoftObjectPtr<UDataTable> SkillEffectTable;

	/** Buff DataTable 软引用 */
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

	mutable TMap<FName, FSkillConfigRow> SkillConfigMap;
	mutable TMap<FName, FSkillEffectRow> SkillEffectMap;
	mutable TMap<FName, FBuffConfigRow> BuffConfigMap;
	mutable TMap<FName, TArray<FName>> SkillToEffectsMap;
	mutable bool bTablesLoaded = false;
};
