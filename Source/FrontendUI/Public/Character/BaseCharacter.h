// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "BaseCharacter.generated.h"

struct FInputActionValue;
struct FHitResult;
struct FGameplayEffectSpec;
struct FGameplayAbilitySpecHandle;
class UAnimMontage;
class UAbilitySystemComponent;
class UGameplayEffect;
class UAbilitySet;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHealthChanged, float, OldHP, float, NewHP, float, Delta, AActor*, Instigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, Instigator);

UCLASS()
class FRONTENDUI_API ABaseCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void Jump() override;
	virtual void StopJumping() override;
	virtual void Landed(const FHitResult& Hit) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void ToggleStrafeMode();

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsStrafeMode() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	FORCEINLINE bool IsJumping() const { return bIsJumping; }

	virtual void Move(const FInputActionValue& Value);
	virtual void Look(const FInputActionValue& Value);

	// ---- 生命值事件 ----

	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnHealthChanged OnMPChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnDeath OnDeath;

	// GAS 接口
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** 轻攻击（包装，等价于 ActivateAbilityByTag(Skill.LightAttack)） */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void LightAttack();

	/** 重攻击（包装，等价于 ActivateAbilityByTag(Skill.HeavyAttack)） */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void HeavyAttack();

	/** 通用技能激活：按 InputTag 查找并激活对应 Ability */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void ActivateAbilityByTag(FGameplayTag Tag);

protected:
	UFUNCTION()
	virtual void OnAttackHit(AActor* HitActor, const FHitResult& HitResult);

	UFUNCTION(BlueprintNativeEvent, Category = "Combat")
	void Die();
	virtual void Die_Implementation();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilitySystemComponent> FallbackASC;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|GAS", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UAbilitySet>> FallbackAbilitySets;

	void BindHealthDelegates();
	void BroadcastInitial();

	void HandleHealthChanged(AActor* InInstigator, AActor* Causer,
		const FGameplayEffectSpec* Spec, float Magnitude, float OldValue, float NewValue);

	void HandleMPChanged(AActor* InInstigator, AActor* Causer,
		const FGameplayEffectSpec* Spec, float Magnitude, float OldValue, float NewValue);
	
	void HandleOutOfHealth(AActor* InInstigator, AActor* Causer,
		const FGameplayEffectSpec* Spec, float Magnitude, float OldValue, float NewValue);

	virtual void AddControllerPitchInput(float Val) override;

	UPROPERTY(EditDefaultsOnly, Category = "Camera|LookLimit", meta = (ClampMin = "-89.0", ClampMax = "89.0"))
	float MinLookPitch = -60.f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera|LookLimit", meta = (ClampMin = "-89.0", ClampMax = "89.0"))
	float MaxLookPitch = 20.f;

	void ApplyStrafeMode(bool bEnable);

	/** 当前发起攻击的技能 ID，OnAttackHit 时读取 */
	FName ActiveSkillID;

	/** 当前发起攻击的技能等级，从 ASC AbilitySpec 查询 */
	int32 ActiveSkillLevel = 1;

	/** 按 FGameplayTag 在 DynamicAbilityTags 中查找对应 Spec 的句柄 */
	FGameplayAbilitySpecHandle FindAbilityHandleByTag(UAbilitySystemComponent* ASC, const FGameplayTag& Tag) const;

	bool bIsJumping = false;
	bool bIsDead    = false;
};
