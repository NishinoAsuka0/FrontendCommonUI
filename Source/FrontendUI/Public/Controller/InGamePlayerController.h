// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InGamePlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class ABaseCharacter;
struct FInputActionValue;

/**
 * 游戏内 PlayerController
 * 负责 EnhancedInput 的绑定和转发，将玩家输入路由到 ABaseCharacter
 */
UCLASS()
class FRONTENDUI_API AInGamePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	// from APlayerController
	/** 绑定 EnhancedInput 动作到处理函数 */
	virtual void SetupInputComponent() override;
	/** Possess 时注册 IMC 到 EnhancedInput 子系统 */
	virtual void OnPossess(APawn* InPawn) override;
	/** UnPossess 时移除 IMC */
	virtual void OnUnPossess() override;
	virtual void BeginPlay() override;  // [DEBUG] 验证类是否被实例化，调试完可删
	// end from APlayerController

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	// 2D 轴：X=左右，Y=前后
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	// 2D 轴：X=Yaw，Y=Pitch
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LightAttackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> HeavyAttackAction;

	/** 移动输入回调：转发给 ABaseCharacter::Move */
	void OnMoveTriggered(const FInputActionValue& Value);
	/** 视角输入回调：转发给 ABaseCharacter::Look */
	void OnLookTriggered(const FInputActionValue& Value);
	/** 跳跃输入回调：调用 Character::Jump */
	void OnJumpStarted();

	/** 轻攻击输入回调：转发给 ABaseCharacter::LightAttack */
	void OnLightAttackStarted();
	/** 重攻击输入回调：转发给 ABaseCharacter::HeavyAttack */
	void OnHeavyAttackStarted();
};
