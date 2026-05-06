// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/SkillGameplayAbility.h"
#include "AbilitySystem/SkillConfigSubsystem.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "FrontendGamePlayTags.h"

void UGA_DataDriven::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	SkillLevel = GetAbilityLevel();

	const FName SkillID = GetSkillIDFromTags();
	if (SkillID.IsNone())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	USkillConfigSubsystem* Subsystem = USkillConfigSubsystem::Get(ActorInfo->AvatarActor.Get());
	if (!Subsystem)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FSkillConfigRow* Config = Subsystem->GetSkillConfig(SkillID);
	if (!Config)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GA_DataDriven] Skill config not found: %s"), *SkillID.ToString());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	CachedSkillConfig = *Config;

	const TArray<FSkillEffectRow> Effects = Subsystem->GetSkillEffects(SkillID);

	AActor* Owner = ActorInfo->AvatarActor.Get();
	UAbilitySystemComponent* OwnerASC = ActorInfo->AbilitySystemComponent.Get();

	for (const FSkillEffectRow& Effect : Effects)
	{
		float EvaluatedValue = Subsystem->EvaluateFormula(
			FString::Printf(TEXT("%.1f + %.1f * Level"), Effect.BaseValue, Effect.ValueScale),
			SkillLevel);

		FGameplayTag EffectType = Effect.EffectType;
		FGameplayTag TargetType = Effect.TargetType;

		if (EffectType.MatchesTag(FrontendGameplayTags::Effect_Damage))
		{
			if (TargetType.MatchesTag(FrontendGameplayTags::Target_Enemy) && TriggerEventData)
			{
				if (AActor* Target = const_cast<AActor*>(TriggerEventData->Target.Get()))
				{
					if (const IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(Target))
					{
						if (UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent())
						{
							FGameplayEffectSpecHandle Spec = Subsystem->MakeEffectSpec(
								Effect, EvaluatedValue, Owner, TargetASC);
							if (Spec.IsValid())
							{
								TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
							}
						}
					}
				}
			}
		}
		else if (EffectType.MatchesTag(FrontendGameplayTags::Effect_Heal))
		{
			if (TargetType.MatchesTag(FrontendGameplayTags::Target_Self))
			{
				FGameplayEffectSpecHandle Spec = Subsystem->MakeEffectSpec(
					Effect, EvaluatedValue, Owner, OwnerASC);
				if (Spec.IsValid())
				{
					OwnerASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
				}
			}
		}
		else if (EffectType.MatchesTag(FrontendGameplayTags::Effect_Buff))
		{
			if (!Effect.BuffID.IsNone())
			{
				const FBuffConfigRow* BuffRow = Subsystem->GetBuffConfig(Effect.BuffID);
				if (BuffRow)
				{
					float ModifierValue = Subsystem->EvaluateFormula(BuffRow->ModifierFormula, SkillLevel);
					float Duration = Subsystem->EvaluateFormula(BuffRow->DurationFormula, SkillLevel);

					UAbilitySystemComponent* TargetASC = nullptr;
					if (TargetType.MatchesTag(FrontendGameplayTags::Target_Self))
					{
						TargetASC = OwnerASC;
					}
					else if (TriggerEventData && TriggerEventData->Target.IsValid())
					{
						const IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(TriggerEventData->Target.Get());
						if (TargetASI)
						{
							TargetASC = TargetASI->GetAbilitySystemComponent();
						}
					}

					if (TargetASC)
					{
						FGameplayEffectSpecHandle Spec = Subsystem->MakeBuffSpec(*BuffRow, ModifierValue, Duration, Owner);
						if (Spec.IsValid())
						{
							TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
						}
					}
				}
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UGA_DataDriven::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FName UGA_DataDriven::GetSkillIDFromTags() const
{
	const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
	if (!Spec) return NAME_None;

	static const FString SkillPrefix(TEXT("Skill."));
	for (const FGameplayTag& Tag : Spec->DynamicAbilityTags)
	{
		FString TagStr = Tag.GetTagName().ToString();
		if (TagStr.StartsWith(SkillPrefix))
		{
			return FName(*TagStr.RightChop(SkillPrefix.Len()));
		}
	}
	return NAME_None;
}
