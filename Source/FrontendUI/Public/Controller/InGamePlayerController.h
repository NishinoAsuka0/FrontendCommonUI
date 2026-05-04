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
 *
 */
UCLASS()
class FRONTENDUI_API AInGamePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	// from APlayerController
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
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

	void OnMoveTriggered(const FInputActionValue& Value);
	void OnLookTriggered(const FInputActionValue& Value);
	void OnJumpStarted();

	void OnLightAttackStarted();
	void OnHeavyAttackStarted();

	// [DEBUG] 临时调试：把 Look 的全部触发事件都接管，反推 IA_MouseLook 的 Trigger 配置，调试完可删
	void OnLookEvtStarted(const FInputActionValue& Value);
	void OnLookEvtOngoing(const FInputActionValue& Value);
	void OnLookEvtCompleted(const FInputActionValue& Value);
	void OnLookEvtCanceled(const FInputActionValue& Value);
	
};
