// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/InGamePlayerController.h"

#include "Character/BaseCharacter.h"
#include "GameFramework/Character.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputActionValue.h"

// [DEBUG] 项目自带的屏幕 + Log 双输出工具，调试完可移除
#include "FrontendDebugHelper.h"


void AInGamePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);

	// [DEBUG] 一次性自检：四个 UPROPERTY 字段必须在 BP_InGamePlayerController 的 Details 面板里赋值，
	//        任何一个为 NULL 都会导致对应的输入完全没绑定
	DebugHelper::Print(FString::Printf(
		TEXT("[InGamePC.Setup] EIC=%s | IMC=%s | Move=%s | Look=%s | Jump=%s"),
		EIC ? TEXT("OK") : TEXT("NULL（InputComponent 不是 UEnhancedInputComponent，需检查 DefaultEngine.ini 的 DefaultInputComponentClass）"),
		DefaultMappingContext ? *DefaultMappingContext->GetName() : TEXT("NULL"),
		MoveAction ? *MoveAction->GetName() : TEXT("NULL"),
		LookAction ? *LookAction->GetName() : TEXT("NULL"),
		JumpAction ? *JumpAction->GetName() : TEXT("NULL")),
		1001, FColor::Cyan);

	if (!EIC)
	{
		return;
	}

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
		// Started=按下瞬间起跳。停止跳跃不再由按键松开驱动，改为 AnimBP 中 UAnimNotify_StopJumping 触发
		EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &AInGamePlayerController::OnJumpStarted);
	}
	if (LightAttackAction)
	{
		EIC->BindAction(LightAttackAction, ETriggerEvent::Started, this, &AInGamePlayerController::OnLightAttackStarted);
	}
	if (HeavyAttackAction)
	{
		EIC->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &AInGamePlayerController::OnHeavyAttackStarted);
	}
}

void AInGamePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// IMC 只属于本地玩家；服务器或 AI 走到这里时跳过
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

	// Re-Possess 不同 Pawn 时清掉旧映射，避免重复叠加
	Subsystem->ClearAllMappings();
	if (DefaultMappingContext)
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	// InputMode 不再在此强切，改由栈上各 ActivatableWidget 通过 GetDesiredInputConfig() 决定。
	// 若栈为空，CommonUI 会回到 PC 默认（Game 模式）。
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

// [DEBUG] 在 Output Log 搜 "[InGamePC.BeginPlay]" 验证 PC 类是否在跑
void AInGamePlayerController::BeginPlay()
{
	Super::BeginPlay();
}


void AInGamePlayerController::OnMoveTriggered(const FInputActionValue& Value)
{
	APawn* P = GetPawn();

	// 转发给 Character：方向矩阵、Pitch 过滤、零输入早退都在 ABaseCharacter::Move 内部
	// Cast 失败（Pawn 不是 ABaseCharacter，例如 SpectatorPawn）时早退，无崩溃
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

void AInGamePlayerController::OnLightAttackStarted()
{
	if (ABaseCharacter* BC = Cast<ABaseCharacter>(GetPawn()))
	{
		BC->LightAttack();
	}
}

void AInGamePlayerController::OnHeavyAttackStarted()
{
	if (ABaseCharacter* BC = Cast<ABaseCharacter>(GetPawn()))
	{
		BC->HeavyAttack();
	}
}
