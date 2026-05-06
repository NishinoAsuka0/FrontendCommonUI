// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/SkillConfigSubsystem.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "FrontendGamePlayTags.h"

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

	auto LoadTable = [](TSoftObjectPtr<UDataTable>& SoftPtr, const TCHAR* TableName) -> UDataTable*
	{
		if (SoftPtr.IsNull()) return nullptr;
		UDataTable* DT = SoftPtr.LoadSynchronous();
		if (!DT)
		{
			UE_LOG(LogTemp, Warning, TEXT("[SkillConfigSubsystem] Failed to load %s"), TableName);
		}
		return DT;
	};

	if (UDataTable* DT = LoadTable(SkillConfigTable, TEXT("DT_SkillConfig")))
	{
		static const FString ContextStr(TEXT("SkillConfigSubsystem::LoadSkillConfig"));
		TArray<FSkillConfigRow*> Rows;
		DT->GetAllRows<FSkillConfigRow>(ContextStr, Rows);
		for (FSkillConfigRow* Row : Rows)
		{
			if (Row) SkillConfigMap.Add(Row->RowName, *Row);
		}
	}

	if (UDataTable* DT = LoadTable(SkillEffectTable, TEXT("DT_SkillEffect")))
	{
		static const FString ContextStr(TEXT("SkillConfigSubsystem::LoadSkillEffect"));
		TArray<FSkillEffectRow*> Rows;
		DT->GetAllRows<FSkillEffectRow>(ContextStr, Rows);
		for (FSkillEffectRow* Row : Rows)
		{
			if (!Row) continue;
			SkillEffectMap.Add(Row->RowName, *Row);
			SkillToEffectsMap.FindOrAdd(Row->SkillID).Add(Row->RowName);
		}
	}

	if (UDataTable* DT = LoadTable(BuffConfigTable, TEXT("DT_BuffConfig")))
	{
		static const FString ContextStr(TEXT("SkillConfigSubsystem::LoadBuffConfig"));
		TArray<FBuffConfigRow*> Rows;
		DT->GetAllRows<FBuffConfigRow>(ContextStr, Rows);
		for (FBuffConfigRow* Row : Rows)
		{
			if (Row) BuffConfigMap.Add(Row->RowName, *Row);
		}
	}
}

const FSkillConfigRow* USkillConfigSubsystem::GetSkillConfig(FName SkillID) const
{
	EnsureTablesLoaded();
	return SkillConfigMap.Find(SkillID);
}

TArray<FSkillEffectRow> USkillConfigSubsystem::GetSkillEffects(FName SkillID) const
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

const FBuffConfigRow* USkillConfigSubsystem::GetBuffConfig(FName BuffID) const
{
	EnsureTablesLoaded();
	return BuffConfigMap.Find(BuffID);
}

float USkillConfigSubsystem::EvaluateFormula(const FString& Formula, int32 Level, int32 Stacks) const
{
	if (Formula.IsEmpty()) return 0.f;
	return FFormulaEvaluator::Evaluate(Formula, Level, Stacks);
}

FGameplayEffectSpecHandle USkillConfigSubsystem::MakeEffectSpec(
	const FSkillEffectRow& EffectRow,
	float EvaluatedValue,
	AActor* SourceActor,
	UAbilitySystemComponent* TargetASC) const
{
	TSubclassOf<UGameplayEffect> GEClass;

	if (EffectRow.EffectType.MatchesTag(FrontendGameplayTags::Effect_Damage))
	{
		GEClass = DamageGEClass;
	}
	else if (EffectRow.EffectType.MatchesTag(FrontendGameplayTags::Effect_Heal))
	{
		GEClass = HealGEClass;
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

FGameplayEffectSpecHandle USkillConfigSubsystem::MakeBuffSpec(
	const FBuffConfigRow& BuffRow,
	float EvaluatedModifierValue,
	float EvaluatedDuration,
	UObject* SourceObject) const
{
	if (!BuffGEClass || !SourceObject) return FGameplayEffectSpecHandle();

	AActor* SourceActor = Cast<AActor>(SourceObject);
	if (!SourceActor) return FGameplayEffectSpecHandle();

	const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(SourceActor);
	if (!ASI) return FGameplayEffectSpecHandle();

	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC) return FGameplayEffectSpecHandle();

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(SourceActor);
	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(BuffGEClass, 1.f, Context);

	if (Spec.IsValid() && EvaluatedDuration > 0.f)
	{
		Spec.Data->SetDuration(EvaluatedDuration, true);
	}

	return Spec;
}
