// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerState/CharacterState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AbilitySet.h"
#include "AbilitySystem/AttributeSet_Base.h"
#include "AttributeSet.h"

ACharacterState::ACharacterState()
{
	// 创建 ASC 子对象，使其与 PlayerState 同生命周期
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
}

UAbilitySystemComponent* ACharacterState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ACharacterState::BeginPlay()
{
	Super::BeginPlay();

	if (!AbilitySystemComponent) return;

	for (const TObjectPtr<UAbilitySet>& Set : AbilitySets)
	{
		if (Set)
		{
			Set->GiveToAbilitySystem(AbilitySystemComponent, this);
		}
	}
}
