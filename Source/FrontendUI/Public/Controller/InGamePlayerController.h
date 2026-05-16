// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "InGamePlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UDataAsset_InputConfig;
class ABaseCharacter;
struct FInputActionValue;


/**
 * 游戏内 PlayerController
 * 负责 EnhancedInput 的绑定和转发，将玩家输入路由到 ABaseCharacter
 *
 * 移动/视角/跳跃：直接绑定到对应处理函数（需要 FInputActionValue 轴值）
 * 技能输入：通过 UDataAsset_InputConfig 映射 InputAction→GameplayTag，
 *           统一由 OnAbilityInputStarted 转发到 BaseCharacter::ActivateAbilityByTag
 */
UCLASS()
class FRONTENDUI_API AInGamePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** InputAction→GameplayTag 映射，遍历绑定所有技能输入 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDataAsset_InputConfig> InputConfig;

	// ---- 移动/视角/跳跃（轴输入，非技能，保持独立绑定）----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;

	void OnMoveTriggered(const FInputActionValue& Value);
	void OnLookTriggered(const FInputActionValue& Value);
	void OnJumpStarted();

	/** 技能输入统一入口：从 InputConfig 查找对应 Tag，转发到 BaseCharacter */
	void OnAbilityInputStarted(FGameplayTag Tag);
};
