// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/SkillGameplayAbility.h"
#include "AbilitySystem/SkillConfigSubsystem.h"
#include "AbilitySystem/AttributeSet_Base.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "FrontendDebugHelper.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/BaseCharacter.h"
#include "GameFramework/Character.h"
#include "Animation/AnimMontage.h"
#include "GameplayEffect.h"
#include "FrontendGamePlayTags.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"

// ============================================================================
// 文件局部辅助函数
// ============================================================================

// 在 CheckCost/CheckCooldown 阶段 CurrentSpecHandle 尚未设置，必须通过 Handle+ASC 查 Spec
static FName GetSkillIDFromHandle(const FGameplayAbilitySpecHandle& Handle, const FGameplayAbilityActorInfo* ActorInfo)
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid()) return NAME_None;
	const FGameplayAbilitySpec* Spec = ActorInfo->AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
	if (!Spec) return NAME_None;
	
	static const FString SkillPrefix(TEXT("Skill."));
	for (const FGameplayTag& Tag : Spec->GetDynamicSpecSourceTags())
	{
		FString TagStr = Tag.GetTagName().ToString();
		if (TagStr.StartsWith(SkillPrefix))
			return FName(*TagStr.RightChop(SkillPrefix.Len()));
	}
	return NAME_None;
}

// CostType Tag → FGameplayAttribute 映射
static FGameplayAttribute GetAttrFromCostType(const FGameplayTag& CostType)
{
	if (CostType == FrontendGameplayTags::Cost_HP)     return UAttributeSet_Base::GetHPAttribute();
	if (CostType == FrontendGameplayTags::Cost_MP)     return UAttributeSet_Base::GetMPAttribute();
	if (CostType == FrontendGameplayTags::Cost_Energy) return UAttributeSet_Base::GetEnergyAttribute();
	return FGameplayAttribute();
}

// SkillID → Cooldown.Skill.{SkillID} Tag
static FGameplayTag GetCooldownTag(FName SkillID)
{
	const FName TagName = FName(*(FString(TEXT("Cooldown.Skill.")) + SkillID.ToString()));
	return FGameplayTag::RequestGameplayTag(TagName, false);
}

// ============================================================================
// UGA_DataDriven
// ============================================================================

void UGA_DataDriven::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// CommitAbility 内部调用 ApplyCost + ApplyCooldown；失败则说明资源不足或仍在冷却
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

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

	ApplySkillEffects(ActorInfo);

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

bool UGA_DataDriven::CheckCost(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	const FName SkillID = GetSkillIDFromHandle(Handle, ActorInfo);
	if (SkillID.IsNone()) return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags);

	USkillConfigSubsystem* Subsystem = USkillConfigSubsystem::Get(ActorInfo->AvatarActor.Get());
	if (!Subsystem) return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags);

	const FSkillConfigRow* Config = Subsystem->GetSkillConfig(SkillID);
	if (!Config || Config->CostFormula.IsEmpty()) return true;

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC) return false;

	const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Handle);
	const float CostValue = FFormulaEvaluator::Evaluate(Config->CostFormula, Spec ? Spec->Level : 1);
	if (CostValue <= 0.f) return true;

	const FGameplayAttribute Attr = GetAttrFromCostType(Config->CostType);
	if (!Attr.IsValid()) return true;

	return ASC->GetNumericAttribute(Attr) >= CostValue;
}

void UGA_DataDriven::ApplyCost(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const FName SkillID = GetSkillIDFromHandle(Handle, ActorInfo);
	if (SkillID.IsNone()) { Super::ApplyCost(Handle, ActorInfo, ActivationInfo); return; }

	USkillConfigSubsystem* Subsystem = USkillConfigSubsystem::Get(ActorInfo->AvatarActor.Get());
	if (!Subsystem) { Super::ApplyCost(Handle, ActorInfo, ActivationInfo); return; }

	const FSkillConfigRow* Config = Subsystem->GetSkillConfig(SkillID);
	if (!Config || Config->CostFormula.IsEmpty()) return;

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC) return;

	const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Handle);
	const float CostValue = FFormulaEvaluator::Evaluate(Config->CostFormula, Spec ? Spec->Level : 1);
	if (CostValue <= 0.f) return;

	const FGameplayAttribute Attr = GetAttrFromCostType(Config->CostType);
	if (!Attr.IsValid()) return;

	// 动态 Instant GE：按 CostType 对应属性扣减 CostValue
	UGameplayEffect* CostGE = NewObject<UGameplayEffect>(GetTransientPackage(), NAME_None, RF_Transient);
	CostGE->DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo Mod;
	Mod.Attribute        = Attr;
	Mod.ModifierOp       = EGameplayModOp::Additive;
	Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(-CostValue));
	CostGE->Modifiers.Add(Mod);

	ASC->ApplyGameplayEffectToSelf(CostGE, 1.f, ASC->MakeEffectContext());
}

bool UGA_DataDriven::CheckCooldown(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	const FName SkillID = GetSkillIDFromHandle(Handle, ActorInfo);
	if (SkillID.IsNone()) return Super::CheckCooldown(Handle, ActorInfo, OptionalRelevantTags);

	const FGameplayTag CooldownTag = GetCooldownTag(SkillID);
	if (!CooldownTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GA_DataDriven] 未注册 Cooldown tag: Cooldown.Skill.%s"), *SkillID.ToString());
		return true;
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC) return true;

	// ASC 持有该 tag 说明技能还在冷却中
	return !ASC->HasMatchingGameplayTag(CooldownTag);
}

void UGA_DataDriven::ApplyCooldown(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const FName SkillID = GetSkillIDFromHandle(Handle, ActorInfo);
	if (SkillID.IsNone()) { Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo); return; }

	USkillConfigSubsystem* Subsystem = USkillConfigSubsystem::Get(ActorInfo->AvatarActor.Get());
	if (!Subsystem) { Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo); return; }

	const FSkillConfigRow* Config = Subsystem->GetSkillConfig(SkillID);
	if (!Config || Config->CooldownFormula.IsEmpty()) return;

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC) return;

	const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Handle);
	const float CooldownValue = FFormulaEvaluator::Evaluate(Config->CooldownFormula, Spec ? Spec->Level : 1);
	if (CooldownValue <= 0.f) return;

	const FGameplayTag CooldownTag = GetCooldownTag(SkillID);
	if (!CooldownTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GA_DataDriven] 未注册 Cooldown tag: Cooldown.Skill.%s"), *SkillID.ToString());
		return;
	}

	// 动态 Duration GE：到期后自动移除 CooldownTag，CheckCooldown 即解除冷却
	UGameplayEffect* CooldownGE = NewObject<UGameplayEffect>(GetTransientPackage(), NAME_None, RF_Transient);
	CooldownGE->DurationPolicy    = EGameplayEffectDurationType::HasDuration;
	CooldownGE->DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(CooldownValue));

	// DynamicGrantedTags 在 Spec 层面添加，兼容 UE 5.5+ 组件化 GE 系统
	FGameplayEffectSpec CooldownSpec(CooldownGE, ASC->MakeEffectContext(), 1.f);
	CooldownSpec.DynamicGrantedTags.AddTag(CooldownTag);
	ASC->ApplyGameplayEffectSpecToSelf(CooldownSpec);
}

FName UGA_DataDriven::GetSkillIDFromTags() const{
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

// ============================================================================
// UGA_DataDriven — 效果应用
// ============================================================================

void UGA_DataDriven::ApplySkillEffects(const FGameplayAbilityActorInfo* ActorInfo)
{
	const FName SkillID = GetSkillIDFromTags();
	if (SkillID.IsNone()) return;

	USkillConfigSubsystem* Subsystem = USkillConfigSubsystem::Get(ActorInfo->AvatarActor.Get());
	if (!Subsystem) return;

	const TArray<FSkillEffectRow> Effects = Subsystem->GetSkillEffects(SkillID);
	if (Effects.IsEmpty()) return;

	// 有 AttackMontage 表示近战靠碰撞判定，Enemy/Area 效果留给 OnAttackHit
	const bool bIsMelee = !CachedSkillConfig.AttackMontage.IsNull();

	for (const FSkillEffectRow& EffectRow : Effects)
	{
		const float Value = EffectRow.BaseValue + EffectRow.ValueScale * SkillLevel;

		if (EffectRow.TargetType.MatchesTag(FrontendGameplayTags::Target_Self))
		{
			ApplyEffectToTarget(EffectRow, Value, ActorInfo, ActorInfo->AvatarActor.Get());
		}
		else if (bIsMelee)
		{
			// 近战技能的 Enemy/Area 效果由武器碰撞器在 OnAttackHit 中施加，此处跳过
			continue;
		}
		else if (EffectRow.TargetType.MatchesTag(FrontendGameplayTags::Target_Enemy))
		{
			const float Range = CachedSkillConfig.CastRange > 0.f ? CachedSkillConfig.CastRange : 200.f;
			if (AActor* Target = FindNearestEnemy(ActorInfo, Range))
			{
				ApplyEffectToTarget(EffectRow, Value, ActorInfo, Target);
			}
		}
		else if (EffectRow.TargetType.MatchesTag(FrontendGameplayTags::Target_Area))
		{
			const float Range = CachedSkillConfig.CastRange > 0.f ? CachedSkillConfig.CastRange : 300.f;
			const TArray<AActor*> Targets = GetEnemiesInRange(ActorInfo, Range);
			for (AActor* Target : Targets)
			{
				ApplyEffectToTarget(EffectRow, Value, ActorInfo, Target);
			}
		}
	}
}

AActor* UGA_DataDriven::FindNearestEnemy(const FGameplayAbilityActorInfo* ActorInfo, float Range) const
{
	const TArray<AActor*> Targets = GetEnemiesInRange(ActorInfo, Range);
	if (Targets.IsEmpty()) return nullptr;

	AActor* Avatar = ActorInfo->AvatarActor.Get();
	if (!Avatar) return nullptr;

	const FVector Origin = Avatar->GetActorLocation();

	AActor* Nearest = nullptr;
	float NearestDistSq = FLT_MAX;
	for (AActor* T : Targets)
	{
		const float DistSq = FVector::DistSquared(Origin, T->GetActorLocation());
		if (DistSq < NearestDistSq)
		{
			NearestDistSq = DistSq;
			Nearest = T;
		}
	}
	return Nearest;
}

TArray<AActor*> UGA_DataDriven::GetEnemiesInRange(const FGameplayAbilityActorInfo* ActorInfo, float Range) const
{
	TArray<AActor*> Result;
	AActor* Avatar = ActorInfo->AvatarActor.Get();
	if (!Avatar) return Result;

	UWorld* World = Avatar->GetWorld();
	if (!World) return Result;

	const FVector Origin = Avatar->GetActorLocation();
	const FVector Forward = Avatar->GetActorForwardVector();

	TArray<FOverlapResult> Overlaps;
	const FCollisionShape Sphere = FCollisionShape::MakeSphere(Range);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Avatar);

	World->OverlapMultiByChannel(
		Overlaps,
		Origin + Forward * Range * 0.5f,
		FQuat::Identity,
		ECC_Pawn,
		Sphere,
		QueryParams);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		ABaseCharacter* OtherChar = Cast<ABaseCharacter>(Overlap.GetActor());
		if (!OtherChar || OtherChar == Avatar) continue;

		if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(OtherChar))
		{
			if (ASI->GetAbilitySystemComponent())
			{
				Result.Add(OtherChar);
			}
		}
	}
	return Result;
}

void UGA_DataDriven::ApplyEffectToTarget(
	const FSkillEffectRow& EffectRow,
	float Value,
	const FGameplayAbilityActorInfo* ActorInfo,
	AActor* Target) const
{
	if (!Target) return;

	USkillConfigSubsystem* Subsystem = USkillConfigSubsystem::Get(ActorInfo->AvatarActor.Get());
	if (!Subsystem) return;

	const IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(Target);
	UAbilitySystemComponent* TargetASC = TargetASI ? TargetASI->GetAbilitySystemComponent() : nullptr;
	if (!TargetASC) return;

	const FGameplayEffectSpecHandle Spec = Subsystem->MakeEffectSpec(
		EffectRow, Value, ActorInfo->AvatarActor.Get(), TargetASC);
	if (Spec.IsValid())
	{
		TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
	}
}

// ============================================================================
// UGA_Passive
// ============================================================================

UGA_Passive::UGA_Passive()
{
	// 被动技能必须实例化，WaitGameplayEvent Task 和 Infinite GE 句柄都需要实例状态
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_Passive::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

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

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CachedLevel = GetAbilityLevel();
	const TArray<FSkillEffectRow> Effects = Subsystem->GetSkillEffects(SkillID);

	for (const FSkillEffectRow& EffectRow : Effects)
	{
		if (EffectRow.TriggerTag.IsValid())
		{
			RegisterTriggerEffect(EffectRow, ASC, CachedLevel);
		}
		else if (EffectRow.DurationType == ESkillEffectDurationType::Infinite)
		{
			ApplyInfiniteEffect(EffectRow, ASC, CachedLevel);
		}
	}
}

void UGA_Passive::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (ASC)
	{
		for (const FActiveGameplayEffectHandle& EffectHandle : AppliedInfiniteEffects)
		{
			if (EffectHandle.IsValid())
			{
				ASC->RemoveActiveGameplayEffect(EffectHandle);
			}
		}
	}
	AppliedInfiniteEffects.Empty();
	TriggerEffectMap.Empty();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FName UGA_Passive::GetSkillIDFromTags() const
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

void UGA_Passive::ApplyInfiniteEffect(const FSkillEffectRow& EffectRow, UAbilitySystemComponent* ASC, int32 Level)
{
	USkillConfigSubsystem* Subsystem = USkillConfigSubsystem::Get(ASC);
	if (!Subsystem) return;

	const float Value = EffectRow.BaseValue + EffectRow.ValueScale * Level;
	const FGameplayEffectSpecHandle Spec = Subsystem->MakePassiveEffectSpec(EffectRow, Value, ASC);
	if (!Spec.IsValid()) return;

	const FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
	if (ActiveHandle.IsValid())
	{
		AppliedInfiniteEffects.Add(ActiveHandle);
	}
}

void UGA_Passive::RegisterTriggerEffect(const FSkillEffectRow& EffectRow, UAbilitySystemComponent* ASC, int32 Level)
{
	TArray<FSkillEffectRow>& TagEffects = TriggerEffectMap.FindOrAdd(EffectRow.TriggerTag);
	const bool bIsFirstForThisTag = TagEffects.IsEmpty();
	TagEffects.Add(EffectRow);

	// 每个唯一 TriggerTag 只注册一个 Task，回调中统一处理该 Tag 下的所有效果
	if (bIsFirstForThisTag)
	{
		UAbilityTask_WaitGameplayEvent* Task = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			EffectRow.TriggerTag,
			nullptr,
			false  // bTriggerOnce=false → 持续监听，每次事件都触发
		);
		Task->EventReceived.AddDynamic(this, &UGA_Passive::OnTriggerEventReceived);
		Task->ReadyForActivation();
	}
}

void UGA_Passive::OnTriggerEventReceived(FGameplayEventData EventData)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return;

	USkillConfigSubsystem* Subsystem = USkillConfigSubsystem::Get(ASC);
	if (!Subsystem) return;

	const TArray<FSkillEffectRow>* Effects = TriggerEffectMap.Find(EventData.EventTag);
	if (!Effects) return;

	for (const FSkillEffectRow& EffectRow : *Effects)
	{
		const float Value = EffectRow.BaseValue + EffectRow.ValueScale * CachedLevel;

		if (EffectRow.DurationType == ESkillEffectDurationType::Infinite)
		{
			// 触发型 Infinite 效果（如"受击后永久获得属性"）
			ApplyInfiniteEffect(EffectRow, ASC, CachedLevel);
		}
		else
		{
			// 触发型即时/有限时长效果：通过 MakeEffectSpec 按 EffectType 路由到对应 GEClass
			AActor* Avatar = GetAvatarActorFromActorInfo();
			const FGameplayEffectSpecHandle Spec = Subsystem->MakeEffectSpec(EffectRow, Value, Avatar, ASC);
			if (Spec.IsValid())
			{
				ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
			}
		}
	}
}
