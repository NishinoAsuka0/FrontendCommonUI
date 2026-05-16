// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/AttributeSet_Base.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "FrontendGamePlayTags.h"

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

bool UAttributeSet_Base::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	// 在 GE 修改属性之前捕获旧值；PreAttributeChange 每次写入都触发，此处每个 GE 只触发一次更精确
	HealthBeforeAttributeChange = GetHP();
	MPBeforeAttributeChange = GetMP();
	return true;
}

void UAttributeSet_Base::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	// IncomingDamage 允许负值（负值 = 回血），不做非负 clamp
	if (Attribute == GetIncomingDamageAttribute()) return;

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

	const FGameplayEffectContextHandle& Context = Data.EffectSpec.GetEffectContext();
	AActor* Instigator = Context.GetOriginalInstigator();
	AActor* Causer     = Context.GetEffectCauser();

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float RawIncoming = GetIncomingDamage();
		if (RawIncoming > 0.f)
		{
			const float NewHP = FMath::Max(GetHP() - RawIncoming, 0.f);
			SetHP(NewHP);

			UE_LOG(LogTemp, Log, TEXT("[Damage] %s 受到 %.0f 点伤害 | HP: %.0f → %.0f"),
				*GetNameSafe(GetOwningActor()), RawIncoming, HealthBeforeAttributeChange, NewHP);

			// 广播受伤 GameplayEvent，驱动被动技能触发（如 Vengeance 受击回血）
			FGameplayEventData EventData;
			EventData.Instigator = Instigator;
			EventData.Target = GetOwningActor();
			EventData.EventMagnitude = RawIncoming;
			GetOwningAbilitySystemComponent()->HandleGameplayEvent(
				FrontendGameplayTags::Trigger_OnDamaged, &EventData);
		}
		else if (RawIncoming < 0.f && !bOutOfHealth)
		{
			// 负值 IncomingDamage = 回血
			const float HealAmount = -RawIncoming;
			const float NewHP = FMath::Min(GetHP() + HealAmount, GetMaxHP());
			SetHP(NewHP);

			UE_LOG(LogTemp, Log, TEXT("[Heal] %s 回复 %.0f 点生命 | HP: %.0f → %.0f"),
				*GetNameSafe(GetOwningActor()), HealAmount, HealthBeforeAttributeChange, NewHP);
		}

		SetIncomingDamage(0.f);
	}
	else if (Data.EvaluatedData.Attribute == GetHPAttribute())
	{
		SetHP(FMath::Clamp(GetHP(), 0.f, GetMaxHP()));
	}
	else if (Data.EvaluatedData.Attribute == GetMPAttribute())
	{
		SetMP(FMath::Clamp(GetMP(), 0.f, GetMaxMP()));
	}
	else if (Data.EvaluatedData.Attribute == GetEnergyAttribute())
	{
		SetEnergy(FMath::Clamp(GetEnergy(), 0.f, GetMaxEnergy()));
	}

	// HP 实际发生变化才广播，避免 Damage=0 时产生噪音事件
	if (GetHP() != HealthBeforeAttributeChange)
	{
		OnHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec,
			Data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHP());
	}
	
	if (GetMP()!= MPBeforeAttributeChange)
	{
		OnMPChanged.Broadcast(Instigator, Causer, &Data.EffectSpec,
			Data.EvaluatedData.Magnitude, MPBeforeAttributeChange, GetMP());
	}

	if ((GetHP() <= 0.f) && !bOutOfHealth)
	{
		OnOutOfHealth.Broadcast(Instigator, Causer, &Data.EffectSpec,
			Data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHP());
	}

	bOutOfHealth = (GetHP() <= 0.f);
}

// ---- Rep callbacks ----

void UAttributeSet_Base::OnRep_HP(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAttributeSet_Base, HP, OldValue);

	const float NewHP = GetHP();
	const float EstimatedMagnitude = NewHP - OldValue.GetCurrentValue();

	// 客户端复制回调：Instigator 信息在客户端不可用，传 nullptr
	OnHealthChanged.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude,
		OldValue.GetCurrentValue(), NewHP);

	if (!bOutOfHealth && NewHP <= 0.f)
	{
		OnOutOfHealth.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude,
			OldValue.GetCurrentValue(), NewHP);
	}

	bOutOfHealth = (NewHP <= 0.f);
}

void UAttributeSet_Base::OnRep_MaxHP(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAttributeSet_Base, MaxHP, OldValue);
}

void UAttributeSet_Base::OnRep_MP(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAttributeSet_Base, MP, OldValue);
	
	const float NewMP = GetMP();
	const float EstimatedMagnitude = NewMP - OldValue.GetCurrentValue();
	
	OnMPChanged.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude,OldValue.GetCurrentValue(),NewMP);
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
