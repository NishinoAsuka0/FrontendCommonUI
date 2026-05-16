// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/SkillConfigSubsystem.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "FrontendGamePlayTags.h"
#include "FrontendSettings/FrontendDeveloperSettings.h"

USkillConfigSubsystem* USkillConfigSubsystem::Get(const UObject* WorldContext)
{
	if (GEngine)
	{
		UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::Assert);
		return UGameInstance::GetSubsystem<USkillConfigSubsystem>(World->GetGameInstance());
	}
	return nullptr;
}

void USkillConfigSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void USkillConfigSubsystem::Deinitialize()
{
	SkillConfigMap.Empty();
	SkillEffectMap.Empty();
	BuffConfigMap.Empty();
	SkillToEffectsMap.Empty();
	bTablesLoaded = false;
	Super::Deinitialize();
}

void USkillConfigSubsystem::EnsureTablesLoaded()
{
	if (bTablesLoaded) return;
	bTablesLoaded = true;

	const UFrontendDeveloperSettings* Settings = GetDefault<UFrontendDeveloperSettings>();
	if (!Settings) return;

	auto LoadTable = [](const TSoftObjectPtr<UDataTable>& SoftPtr, const TCHAR* TableName) -> UDataTable*
	{
		if (SoftPtr.IsNull()) return nullptr;
		UDataTable* DT = SoftPtr.LoadSynchronous();
		if (!DT)
		{
			UE_LOG(LogTemp, Warning, TEXT("[SkillConfigSubsystem] Failed to load %s"), TableName);
		}
		return DT;
	};

	if (UDataTable* DT = LoadTable(Settings->SkillConfigTable, TEXT("DT_SkillConfig")))
	{
		for (const TPair<FName, uint8*>& Pair : DT->GetRowMap())
		{
			if (FSkillConfigRow* Row = reinterpret_cast<FSkillConfigRow*>(Pair.Value))
			{
				SkillConfigMap.Add(Pair.Key, *Row);
			}
		}
	}

	if (UDataTable* DT = LoadTable(Settings->SkillEffectTable, TEXT("DT_SkillEffect")))
	{
		for (const TPair<FName, uint8*>& Pair : DT->GetRowMap())
		{
			if (FSkillEffectRow* Row = reinterpret_cast<FSkillEffectRow*>(Pair.Value))
			{
				SkillEffectMap.Add(Pair.Key, *Row);
				SkillToEffectsMap.FindOrAdd(Row->SkillID).Add(Pair.Key);
			}
		}
	}

	if (UDataTable* DT = LoadTable(Settings->BuffConfigTable, TEXT("DT_BuffConfig")))
	{
		for (const TPair<FName, uint8*>& Pair : DT->GetRowMap())
		{
			if (FBuffConfigRow* Row = reinterpret_cast<FBuffConfigRow*>(Pair.Value))
			{
				BuffConfigMap.Add(Pair.Key, *Row);
			}
		}
	}
}

const FSkillConfigRow* USkillConfigSubsystem::GetSkillConfig(FName SkillID)
{
	EnsureTablesLoaded();
	return SkillConfigMap.Find(SkillID);
}

TArray<FSkillEffectRow> USkillConfigSubsystem::GetSkillEffects(FName SkillID)
{
	EnsureTablesLoaded();

	TArray<FSkillEffectRow> Result;
	if (const TArray<FName>* EffectIDs = SkillToEffectsMap.Find(SkillID))
	{
		Result.Reserve(EffectIDs->Num());
		for (const FName& EffectID : *EffectIDs)
		{
			if (const FSkillEffectRow* Row = SkillEffectMap.Find(EffectID))
			{
				Result.Add(*Row);
			}
		}
	}
	return Result;
}

const FBuffConfigRow* USkillConfigSubsystem::GetBuffConfig(FName BuffID)
{
	EnsureTablesLoaded();
	return BuffConfigMap.Find(BuffID);
}

float USkillConfigSubsystem::EvaluateFormula(const FString& Formula, int32 Level, int32 Stacks) const
{
	if (Formula.IsEmpty()) return 0.f;
	return FFormulaEvaluator::Evaluate(Formula, Level, Stacks);
}

float USkillConfigSubsystem::GetSkillCooldownTimeRemaining(FName SkillID, UAbilitySystemComponent* ASC) const
{
	if (!ASC || SkillID.IsNone()) return 0.f;

	const FName TagName = FName(*(FString(TEXT("Cooldown.Skill.")) + SkillID.ToString()));
	const FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(TagName, false);
	if (!CooldownTag.IsValid()) return 0.f;

	// MakeQuery_MatchAnyOwningTags 会检查 Spec::GetAllGrantedTags()，包含 DynamicGrantedTags
	const FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(
		FGameplayTagContainer(CooldownTag));

	TArray<float> Times = ASC->GetActiveEffectsTimeRemaining(Query);
	return Times.Num() > 0 ? FMath::Max(0.f, Times[0]) : 0.f;
}

FGameplayEffectSpecHandle USkillConfigSubsystem::MakeEffectSpec(
	const FSkillEffectRow& EffectRow,
	float EvaluatedValue,
	AActor* SourceActor,
	UAbilitySystemComponent* TargetASC) const
{
	const UFrontendDeveloperSettings* Settings = GetDefault<UFrontendDeveloperSettings>();
	if (!Settings) return FGameplayEffectSpecHandle();

	TSubclassOf<UGameplayEffect> GEClass;

	if (EffectRow.EffectType.MatchesTag(FrontendGameplayTags::Effect_Damage))
	{
		GEClass = Settings->DamageGEClass;
	}
	else if (EffectRow.EffectType.MatchesTag(FrontendGameplayTags::Effect_Heal))
	{
		// 复用 DamageGEClass：负值 IncomingDamage 在 AttributeSet 中被当作回血处理
		GEClass = Settings->DamageGEClass;
		EvaluatedValue = -FMath::Abs(EvaluatedValue);
	}

	if (!GEClass || !SourceActor) return FGameplayEffectSpecHandle();

	const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(SourceActor);
	UAbilitySystemComponent* SourceASC = ASI ? ASI->GetAbilitySystemComponent() : nullptr;
	if (!SourceASC) return FGameplayEffectSpecHandle();

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(SourceActor);
	const FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(GEClass, 1.f, Context);

	if (Spec.IsValid() && EffectRow.ValueAttribute.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(EffectRow.ValueAttribute, EvaluatedValue);
	}

	return Spec;
}

FGameplayEffectSpecHandle USkillConfigSubsystem::MakePassiveEffectSpec(
	const FSkillEffectRow& EffectRow,
	float EvaluatedValue,
	UAbilitySystemComponent* OwnerASC) const
{
	const UFrontendDeveloperSettings* Settings = GetDefault<UFrontendDeveloperSettings>();
	if (!Settings || !OwnerASC) return FGameplayEffectSpecHandle();

	TSubclassOf<UGameplayEffect> GEClass;
	if (EffectRow.EffectType.MatchesTag(FrontendGameplayTags::Effect_Heal))
	{
		GEClass = Settings->HealGEClass;
		EvaluatedValue = -FMath::Abs(EvaluatedValue);
	}
	else
	{
		GEClass = Settings->PassiveGEClass;
	}

	if (!GEClass) return FGameplayEffectSpecHandle();

	FGameplayEffectContextHandle Context = OwnerASC->MakeEffectContext();
	const FGameplayEffectSpecHandle Spec = OwnerASC->MakeOutgoingSpec(GEClass, 1.f, Context);

	if (Spec.IsValid() && EffectRow.ValueAttribute.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(EffectRow.ValueAttribute, EvaluatedValue);
	}

	return Spec;
}

FGameplayEffectSpecHandle USkillConfigSubsystem::MakeBuffSpec(
	const FBuffConfigRow& BuffRow,
	float EvaluatedModifierValue,
	float EvaluatedDuration,
	UObject* SourceObject) const
{
	const UFrontendDeveloperSettings* Settings = GetDefault<UFrontendDeveloperSettings>();
	if (!Settings || !Settings->BuffGEClass || !SourceObject) return FGameplayEffectSpecHandle();

	AActor* SourceActor = Cast<AActor>(SourceObject);
	if (!SourceActor) return FGameplayEffectSpecHandle();

	const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(SourceActor);
	if (!ASI) return FGameplayEffectSpecHandle();

	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC) return FGameplayEffectSpecHandle();

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(SourceActor);
	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(Settings->BuffGEClass, 1.f, Context);

	if (Spec.IsValid() && EvaluatedDuration > 0.f)
	{
		Spec.Data->SetDuration(EvaluatedDuration, true);
	}

	return Spec;
}
