// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/AttributeSet_Base.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UAttributeSet_Base::UAttributeSet_Base()
{
	MaxHP.SetBaseValue(100.f);
	MaxHP.SetCurrentValue(100.f);

	MaxMP.SetBaseValue(100.f);
	MaxMP.SetCurrentValue(100.f);

	MaxEnergy.SetBaseValue(100.f);
	MaxEnergy.SetCurrentValue(100.f);

	HP.SetBaseValue(MaxHP.GetBaseValue());
	HP.SetCurrentValue(MaxHP.GetCurrentValue());

	MP.SetBaseValue(MaxMP.GetBaseValue());
	MP.SetCurrentValue(MaxMP.GetCurrentValue());

	Energy.SetBaseValue(MaxEnergy.GetBaseValue());
	Energy.SetCurrentValue(MaxEnergy.GetCurrentValue());
}

void UAttributeSet_Base::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAttributeSet_Base, HP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAttributeSet_Base, MaxHP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAttributeSet_Base, MP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAttributeSet_Base, MaxMP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAttributeSet_Base, Energy, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAttributeSet_Base, MaxEnergy, COND_None, REPNOTIFY_Always);
}

void UAttributeSet_Base::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	// 下限统一为 0
	NewValue = FMath::Max(NewValue, 0.f);

	if (Attribute == GetHPAttribute())
	{
		NewValue = FMath::Min(NewValue, MaxHP.GetBaseValue());
	}
	else if (Attribute == GetMPAttribute())
	{
		NewValue = FMath::Min(NewValue, MaxMP.GetBaseValue());
	}
	else if (Attribute == GetEnergyAttribute())
	{
		NewValue = FMath::Min(NewValue, MaxEnergy.GetBaseValue());
	}
}

void UAttributeSet_Base::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// 当前值也受上限约束
	if (Attribute == GetHPAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, MaxHP.GetCurrentValue());
	}
	else if (Attribute == GetMPAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, MaxMP.GetCurrentValue());
	}
	else if (Attribute == GetEnergyAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, MaxEnergy.GetCurrentValue());
	}
}

void UAttributeSet_Base::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float Damage = IncomingDamage.GetCurrentValue();
		if (Damage > 0.f)
		{
			const float OldHP = HP.GetCurrentValue();
			const float NewHP = FMath::Max(OldHP - Damage, 0.f);
			HP.SetCurrentValue(NewHP);

			AActor* Owner = GetOwningActor();
			UE_LOG(LogTemp, Log, TEXT("[Damage] %s 受到 %.0f 点伤害 | HP: %.0f → %.0f (Max: %.0f)"),
				Owner ? *Owner->GetName() : TEXT("Unknown"),
				Damage, OldHP, NewHP, MaxHP.GetCurrentValue());
		}

		IncomingDamage.SetBaseValue(0.f);
		IncomingDamage.SetCurrentValue(0.f);
		return;
	}

	if (Data.EvaluatedData.Attribute == GetHPAttribute())
	{
		HP.SetCurrentValue(FMath::Clamp(HP.GetCurrentValue(), 0.f, MaxHP.GetCurrentValue()));
	}
	else if (Data.EvaluatedData.Attribute == GetMPAttribute())
	{
		MP.SetCurrentValue(FMath::Clamp(MP.GetCurrentValue(), 0.f, MaxMP.GetCurrentValue()));
	}
	else if (Data.EvaluatedData.Attribute == GetEnergyAttribute())
	{
		Energy.SetCurrentValue(FMath::Clamp(Energy.GetCurrentValue(), 0.f, MaxEnergy.GetCurrentValue()));
	}
}

// ---- Rep callbacks ----

void UAttributeSet_Base::OnRep_HP(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAttributeSet_Base, HP, OldValue);
}

void UAttributeSet_Base::OnRep_MaxHP(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAttributeSet_Base, MaxHP, OldValue);
}

void UAttributeSet_Base::OnRep_MP(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAttributeSet_Base, MP, OldValue);
}

void UAttributeSet_Base::OnRep_MaxMP(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAttributeSet_Base, MaxMP, OldValue);
}

void UAttributeSet_Base::OnRep_Energy(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAttributeSet_Base, Energy, OldValue);
}

void UAttributeSet_Base::OnRep_MaxEnergy(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAttributeSet_Base, MaxEnergy, OldValue);
}
