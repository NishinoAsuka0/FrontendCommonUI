// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet_Base.generated.h"

// 用宏替代重复的 Get/Set/Init/RepNotify 样板
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

struct FGameplayEffectSpec;

// 内部 C++ 委托，不暴露给蓝图，供绑定方（如 ABaseCharacter）主动订阅
DECLARE_MULTICAST_DELEGATE_SixParams(FOnAttributeHealthEvent,
	AActor* /*Instigator*/, AActor* /*Causer*/,
	const FGameplayEffectSpec* /*Spec*/, float /*Magnitude*/,
	float /*OldValue*/, float /*NewValue*/);

/**
 * 基础属性集：HP / MP / Energy，各自带 Max。
 *
 * 放入 ACharacterState 的 GrantedAttributes 中由 AbilitySet 自动创建。
 */
UCLASS()
class FRONTENDUI_API UAttributeSet_Base : public UAttributeSet
{
	GENERATED_BODY()

public:
	UAttributeSet_Base();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// ---- HP ----
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_HP, Category = "Attributes")
	FGameplayAttributeData HP;
	ATTRIBUTE_ACCESSORS(UAttributeSet_Base, HP)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHP, Category = "Attributes")
	FGameplayAttributeData MaxHP;
	ATTRIBUTE_ACCESSORS(UAttributeSet_Base, MaxHP)

	// ---- MP ----
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MP, Category = "Attributes")
	FGameplayAttributeData MP;
	ATTRIBUTE_ACCESSORS(UAttributeSet_Base, MP)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMP, Category = "Attributes")
	FGameplayAttributeData MaxMP;
	ATTRIBUTE_ACCESSORS(UAttributeSet_Base, MaxMP)

	// ---- Energy ----
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Energy, Category = "Attributes")
	FGameplayAttributeData Energy;
	ATTRIBUTE_ACCESSORS(UAttributeSet_Base, Energy)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxEnergy, Category = "Attributes")
	FGameplayAttributeData MaxEnergy;
	ATTRIBUTE_ACCESSORS(UAttributeSet_Base, MaxEnergy)

	// ---- Damage（元属性：非复制，GE 传入伤害值，PostGameplayEffectExecute 处理后归零）----
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Meta")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UAttributeSet_Base, IncomingDamage);

	// HP 发生任何变化时广播
	mutable FOnAttributeHealthEvent OnHealthChanged;
	
	// MP 发生变化时广播
	
	mutable FOnAttributeHealthEvent OnMPChanged;

	// HP 首次降至 0 时广播（bOutOfHealth 保证同一次死亡只触发一次）
	mutable FOnAttributeHealthEvent OnOutOfHealth;

protected:
	UFUNCTION()
	void OnRep_HP(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHP(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MP(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxMP(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Energy(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxEnergy(const FGameplayAttributeData& OldValue);

private:
	// PreGameplayEffectExecute 里捕获，比 PreAttributeChange 更精确：每个 GE 只捕获一次
	float HealthBeforeAttributeChange = 0.f;
	
	float MPBeforeAttributeChange = 0.f;

	// 防止 HP 已为 0 时多帧 GE 重复触发 OnOutOfHealth
	bool bOutOfHealth = false;
};
