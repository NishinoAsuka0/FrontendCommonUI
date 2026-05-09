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
#include "AbilitySystem/AbilitySet.h"
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

	if (FallbackASC)
	{
		for (const TObjectPtr<UAbilitySet>& Set : FallbackAbilitySets)
		{
			if (Set) Set->GiveToAbilitySystem(FallbackASC, this);
		}
	}
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
	}
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
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	USkillConfigSubsystem* SkillSub = USkillConfigSubsystem::Get(this);
	if (!SkillSub) return;

	const FSkillConfigRow* Config = SkillSub->GetSkillConfig(LightAttackSkillID);
	if (!Config || Config->AbilityTags.IsEmpty()) return;

	ActiveSkillID = LightAttackSkillID;
	ASC->TryActivateAbilitiesByTag(Config->AbilityTags);
}

void ABaseCharacter::HeavyAttack()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	USkillConfigSubsystem* SkillSub = USkillConfigSubsystem::Get(this);
	if (!SkillSub) return;

	const FSkillConfigRow* Config = SkillSub->GetSkillConfig(HeavyAttackSkillID);
	if (!Config || Config->AbilityTags.IsEmpty()) return;

	ActiveSkillID = HeavyAttackSkillID;
	ASC->TryActivateAbilitiesByTag(Config->AbilityTags);
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

		const float Damage = FFormulaEvaluator::Evaluate(
			FString::Printf(TEXT("%.1f + %.1f * Level"), Effect.BaseValue, Effect.ValueScale), 1);

		FGameplayEffectSpecHandle Spec = SkillSubsystem->MakeEffectSpec(Effect, Damage, this, TargetASC);
		if (Spec.IsValid())
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
	}
}
