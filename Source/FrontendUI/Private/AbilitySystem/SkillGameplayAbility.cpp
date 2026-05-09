// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/SkillGameplayAbility.h"
#include "AbilitySystem/SkillConfigSubsystem.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimMontage.h"

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

	if (!CachedSkillConfig.AttackMontage.IsNull())
	{
		if (UAnimMontage* Montage = CachedSkillConfig.AttackMontage.LoadSynchronous())
		{
			if (ACharacter* AvatarChar = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
			{
				AvatarChar->PlayAnimMontage(Montage);
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
	for (const FGameplayTag& Tag : Spec->GetDynamicSpecSourceTags())
	{
		FString TagStr = Tag.GetTagName().ToString();
		if (TagStr.StartsWith(SkillPrefix))
		{
			return FName(*TagStr.RightChop(SkillPrefix.Len()));
		}
	}
	return NAME_None;
}
