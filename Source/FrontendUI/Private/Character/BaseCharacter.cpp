// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BaseCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "InputActionValue.h"

#include "Character/WeaponColliderComponent.h"
#include "Animation/AnimMontage.h"

// GAS
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/AbilitySet.h"
#include "AbilitySystem/AttributeSet_Base.h"
#include "PlayerState/CharacterState.h"
#include "GameplayEffect.h"

#include "AbilitySystem/SkillConfigSubsystem.h"
#include "AbilitySystem/SkillConfigTypes.h"
#include "FrontendGamePlayTags.h"

#include "FrontendDebugHelper.h"


// =====================================================================================
// 构造函数
// =====================================================================================
ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw   = false;
	bUseControllerRotationRoll  = false;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->bOrientRotationToMovement = true;
	MoveComp->RotationRate = FRotator(0.f, 540.f, 0.f);
	MoveComp->JumpZVelocity = 700.f;
	MoveComp->AirControl = 0.35f;
	MoveComp->MaxWalkSpeed = 500.f;
	MoveComp->MinAnalogWalkSpeed = 20.f;
	MoveComp->BrakingDecelerationWalking = 2000.f;

	FallbackASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("FallbackASC"));
}

// =====================================================================================
// 生命值事件
// =====================================================================================
void ABaseCharacter::BindHealthDelegates()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	if (const UAttributeSet_Base* AttrSet = ASC->GetSet<UAttributeSet_Base>())
	{
		if (!AttrSet->OnHealthChanged.IsBoundToObject(this))
		{
			AttrSet->OnHealthChanged.AddUObject(this, &ABaseCharacter::HandleHealthChanged);
			AttrSet->OnOutOfHealth.AddUObject(this, &ABaseCharacter::HandleOutOfHealth);
			AttrSet->OnMPChanged.AddUObject(this, &ABaseCharacter::HandleMPChanged);
		}
	}
}

void ABaseCharacter::BroadcastInitial()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	if (const UAttributeSet_Base* AttrSet = ASC->GetSet<UAttributeSet_Base>())
	{
		const float InitHP = AttrSet->GetHP();
		const float InitMP = AttrSet->GetMP();
		HandleHealthChanged(nullptr, nullptr, nullptr, 0.f, InitHP, InitHP);
		HandleMPChanged(nullptr, nullptr, nullptr, 0.f, InitMP, InitMP);
	}
}

void ABaseCharacter::HandleHealthChanged(AActor* InInstigator, AActor* Causer,
	const FGameplayEffectSpec* Spec, float Magnitude, float OldValue, float NewValue)
{
	const float Delta = NewValue - OldValue;
	OnHealthChanged.Broadcast(OldValue, NewValue, Delta, InInstigator);

	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
		FString::Printf(TEXT("[HP] %.0f → %.0f (%+.0f)"), OldValue, NewValue, Delta));
}


void ABaseCharacter::HandleMPChanged(AActor* InInstigator, AActor* Causer, const FGameplayEffectSpec* Spec,
	float Magnitude, float OldValue, float NewValue)
{
	const float Delta = NewValue - OldValue;
	OnMPChanged.Broadcast(OldValue, NewValue, Delta, InInstigator);

	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue,
		FString::Printf(TEXT("[MP] %.0f → %.0f (%+.0f)"), OldValue, NewValue, Delta));
}

void ABaseCharacter::HandleOutOfHealth(AActor* InInstigator, AActor* Causer,
                                       const FGameplayEffectSpec* Spec, float Magnitude, float OldValue, float NewValue)
{
	Die();
	OnDeath.Broadcast(InInstigator);
}

void ABaseCharacter::Die_Implementation()
{
	if (bIsDead) return;
	bIsDead = true;

	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->DisableInput(PC);
	}
}

// =====================================================================================
// Strafe
// =====================================================================================
void ABaseCharacter::ToggleStrafeMode()
{
	ApplyStrafeMode(!IsStrafeMode());
}

bool ABaseCharacter::IsStrafeMode() const
{
	return bUseControllerRotationYaw;
}

void ABaseCharacter::ApplyStrafeMode(bool bEnable)
{
	bUseControllerRotationYaw = bEnable;
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = !bEnable;
	}
}

// =====================================================================================
// 基础移动
// =====================================================================================
void ABaseCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (!Controller || Axis.IsNearlyZero()) return;

	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FRotationMatrix RotMatrix(YawRotation);
	AddMovementInput(RotMatrix.GetUnitAxis(EAxis::X), Axis.Y);
	AddMovementInput(RotMatrix.GetUnitAxis(EAxis::Y), Axis.X);
}

void ABaseCharacter::Look(const FInputActionValue& Value)
{
	if (!Controller) return;
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

// =====================================================================================
// Pitch
// =====================================================================================
void ABaseCharacter::AddControllerPitchInput(float Val)
{
	if (Val == 0.f || !Controller)
	{
		Super::AddControllerPitchInput(Val);
		return;
	}
	const float CurrentPitch = FRotator::NormalizeAxis(Controller->GetControlRotation().Pitch);
	const bool bWillLookUp   = (Val > 0.f);
	const bool bWillLookDown = (Val < 0.f);
	if (bWillLookUp && CurrentPitch <= MinLookPitch) return;
	if (bWillLookDown && CurrentPitch >= MaxLookPitch) return;
	Super::AddControllerPitchInput(Val);
}

// =====================================================================================
// BeginPlay / Possess
// =====================================================================================
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (UWeaponColliderComponent* WC = FindComponentByClass<UWeaponColliderComponent>())
	{
		WC->OnWeaponHit.AddDynamic(this, &ABaseCharacter::OnAttackHit);
	}

	BindHealthDelegates();
	BroadcastInitial();
}

void ABaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (ACharacterState* PS = GetPlayerState<ACharacterState>())
	{
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);
	}
	else if (FallbackASC)
	{
		FallbackASC->InitAbilityActorInfo(this, this);
		for (const TObjectPtr<UAbilitySet>& Set : FallbackAbilitySets)
		{
			if (Set) Set->GiveToAbilitySystem(FallbackASC, this);
		}
	}
	BindHealthDelegates();
}

void ABaseCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (ACharacterState* PS = GetPlayerState<ACharacterState>())
	{
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);
	}
	else if (FallbackASC)
	{
		FallbackASC->InitAbilityActorInfo(this, this);
	}
	BindHealthDelegates();
}

void ABaseCharacter::Jump()
{
	Super::Jump();
	bIsJumping = true;
}

void ABaseCharacter::StopJumping()
{
	Super::StopJumping();
	bIsJumping = false;
}

void ABaseCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	StopJumping();
}

// =====================================================================================
// 战斗
// =====================================================================================
UAbilitySystemComponent* ABaseCharacter::GetAbilitySystemComponent() const
{
	if (const ACharacterState* PS = GetPlayerState<ACharacterState>())
	{
		if (UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		{
			return ASC;
		}
	}
	return FallbackASC;
}

void ABaseCharacter::LightAttack()
{
	ActivateAbilityByTag(FrontendGameplayTags::Skill_LightAttack);
}

void ABaseCharacter::HeavyAttack()
{
	ActivateAbilityByTag(FrontendGameplayTags::Skill_HeavyAttack);
}

void ABaseCharacter::ActivateAbilityByTag(FGameplayTag Tag)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		DebugHelper::Print("ActivateAbilityByTag FAIL: ASC is NULL", 2, FColor::Red);
		return;
	}

	const FGameplayAbilitySpecHandle Handle = FindAbilityHandleByTag(ASC, Tag);
	if (!Handle.IsValid())
	{
		DebugHelper::Print(FString::Printf(TEXT("ActivateAbilityByTag FAIL: no ability found for Tag=%s"), *Tag.GetTagName().ToString()), 2, FColor::Red);
		return;
	}

	const FString TagStr = Tag.GetTagName().ToString();
	const int32 DotIndex = TagStr.Find(TEXT("."));
	ActiveSkillID = (DotIndex != INDEX_NONE) ? FName(*TagStr.Mid(DotIndex + 1)) : FName(*TagStr);

	if (const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Handle))
	{
		ActiveSkillLevel = Spec->Level;
	}

	const bool bActivated = ASC->TryActivateAbility(Handle);
	if (!bActivated)
	{
		DebugHelper::Print(FString::Printf(TEXT("ActivateAbilityByTag FAIL: TryActivateAbility returned false for Tag=%s, SkillID=%s, Level=%d"),
			*TagStr, *ActiveSkillID.ToString(), ActiveSkillLevel), 2, FColor::Red);
	}
}

void ABaseCharacter::OnAttackHit(AActor* HitActor, const FHitResult& HitResult)
{
	if (!HitActor) return;

	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(HitActor);
	UAbilitySystemComponent* TargetASC = ASI ? ASI->GetAbilitySystemComponent() : nullptr;
	UAbilitySystemComponent* MyASC = GetAbilitySystemComponent();

	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan,
		FString::Printf(TEXT("[Hit] %s → ASC=%s"),
			*HitActor->GetName(),
			TargetASC ? *TargetASC->GetOwner()->GetName() : TEXT("NULL")));

	if (!TargetASC || !MyASC || ActiveSkillID.IsNone()) return;

	USkillConfigSubsystem* SkillSubsystem = USkillConfigSubsystem::Get(this);
	if (!SkillSubsystem) return;

	const TArray<FSkillEffectRow> Effects = SkillSubsystem->GetSkillEffects(ActiveSkillID);
	for (const FSkillEffectRow& Effect : Effects)
	{
		if (!Effect.EffectType.MatchesTag(FrontendGameplayTags::Effect_Damage)) continue;

		const float Damage = Effect.BaseValue + Effect.ValueScale * static_cast<float>(ActiveSkillLevel);

		FGameplayEffectSpecHandle Spec = SkillSubsystem->MakeEffectSpec(Effect, Damage, this, TargetASC);
		if (Spec.IsValid())
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
	}
}

FGameplayAbilitySpecHandle ABaseCharacter::FindAbilityHandleByTag(UAbilitySystemComponent* ASC, const FGameplayTag& Tag) const
{
	if (!ASC) return {};

	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.GetDynamicSpecSourceTags().HasTagExact(Tag))
		{
			return Spec.Handle;
		}
	}
	return {};
}
