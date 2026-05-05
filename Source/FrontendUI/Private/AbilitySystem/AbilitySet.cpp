// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/AbilitySet.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayEffect.h"
#include "AttributeSet.h"

void UAbilitySet::GiveToAbilitySystem(UAbilitySystemComponent* ASC, UObject* SourceObject) const
{
	if (!ASC)
	{
		return;
	}

	// 1) 授予 Ability
	for (const FAbilitySet_GameplayAbility& Entry : GrantedAbilities)
	{
		if (!Entry.Ability)
		{
			continue;
		}

		FGameplayAbilitySpec Spec(Entry.Ability, Entry.AbilityLevel);
		Spec.SourceObject = SourceObject;

		if (Entry.InputTag.IsValid())
		{
			Spec.DynamicAbilityTags.AddTag(Entry.InputTag);
		}

		ASC->GiveAbility(Spec);
	}

	// 3) 应用 GameplayEffect（立即生效，通常用于属性初始化）
	for (const FAbilitySet_GameplayEffect& Entry : GrantedEffects)
	{
		if (!Entry.GameplayEffect)
		{
			continue;
		}

		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddSourceObject(SourceObject);

		const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(Entry.GameplayEffect, Entry.EffectLevel, Context);
		if (Spec.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
	}
}
