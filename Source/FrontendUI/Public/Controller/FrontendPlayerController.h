// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FrontendPlayerController.generated.h"

/**
 * 前端 UI 专用 PlayerController
 * Possess 时自动切换到场景中带 "Default" 标签的 CameraActor
 */
UCLASS()
class FRONTENDUI_API AFrontendPlayerController : public APlayerController
{
	GENERATED_BODY()
protected:
	/** Possess Pawn 时自动查找带 "Default" Tag 的 CameraActor 并切换视角 */
	virtual void OnPossess(APawn* aPawn) override;
};
