// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "CharacterState.generated.h"

class UAbilitySystemComponent;
class UAbilitySet;

/**
 * 角色 PlayerState
 * 持有 ASC（AbilitySystemComponent）并在 BeginPlay 时授予 AbilitySet 中配置的能力
 */
UCLASS()
class FRONTENDUI_API ACharacterState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ACharacterState();

	/** 返回持有的 AbilitySystemComponent */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	/** BeginPlay 时授予 AbilitySet 并初始化 ASC 的 ActorInfo */
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	/** AbilitySet 列表：BeginPlay 时逐一授予 ASC */
	UPROPERTY(EditDefaultsOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UAbilitySet>> AbilitySets;
};
