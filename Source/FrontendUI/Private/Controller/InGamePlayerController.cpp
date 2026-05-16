// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/InGamePlayerController.h"

#include "Character/BaseCharacter.h"
#include "Input/InputConfig.h"
#include "GameFramework/Character.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputActionValue.h"

#include "FrontendDebugHelper.h"


void AInGamePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);

	DebugHelper::Print(FString::Printf(
		TEXT("[InGamePC.Setup] EIC=%s | IMC=%s | Move=%s | Look=%s | Jump=%s | InputConfig=%s"),
		EIC ? TEXT("OK") : TEXT("NULL"),
		DefaultMappingContext ? *DefaultMappingContext->GetName() : TEXT("NULL"),
		MoveAction ? *MoveAction->GetName() : TEXT("NULL"),
		LookAction ? *LookAction->GetName() : TEXT("NULL"),
		JumpAction ? *JumpAction->GetName() : TEXT("NULL"),
		InputConfig ? *InputConfig->GetName() : TEXT("NULL")),
		1001, FColor::Cyan);

	if (!EIC)
	{
		return;
	}

	// ---- 移动/视角/跳跃（轴输入，保持独立绑定）----
	if (MoveAction)
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AInGamePlayerController::OnMoveTriggered);
	}
	if (LookAction)
	{
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AInGamePlayerController::OnLookTriggered);
	}
	if (JumpAction)
	{
		EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &AInGamePlayerController::OnJumpStarted);
	}

	// ---- 技能输入（InputConfig 驱动，遍历绑定）----
	if (InputConfig)
	{
		for (const auto& Pair : InputConfig->AbilityInputMap)
		{
			if (Pair.Key)
			{
				EIC->BindAction(Pair.Key, ETriggerEvent::Started, this, &ThisClass::OnAbilityInputStarted, Pair.Value);
			}
		}
	}
}

void AInGamePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!IsLocalController())
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!Subsystem)
	{
		return;
	}

	Subsystem->ClearAllMappings();
	if (DefaultMappingContext)
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void AInGamePlayerController::OnUnPossess()
{
	if (IsLocalController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->RemoveMappingContext(DefaultMappingContext);
			}
		}
	}

	Super::OnUnPossess();
}

void AInGamePlayerController::BeginPlay()
{
	Super::BeginPlay();
}


void AInGamePlayerController::OnMoveTriggered(const FInputActionValue& Value)
{
	APawn* P = GetPawn();

	if (ABaseCharacter* ControlledCharacter = Cast<ABaseCharacter>(P))
	{
		ControlledCharacter->Move(Value);
	}
}

void AInGamePlayerController::OnLookTriggered(const FInputActionValue& Value)
{
	APawn* P = GetPawn();

	if (ABaseCharacter* ControlledCharacter = Cast<ABaseCharacter>(P))
	{
		ControlledCharacter->Look(Value);
	}
}

void AInGamePlayerController::OnJumpStarted()
{
	if (ACharacter* ControlledCharacter = GetCharacter())
	{
		ControlledCharacter->Jump();
	}
}

void AInGamePlayerController::OnAbilityInputStarted(FGameplayTag Tag)
{
	if (ABaseCharacter* BC = Cast<ABaseCharacter>(GetPawn()))
	{
		BC->ActivateAbilityByTag(Tag);
	}
}
