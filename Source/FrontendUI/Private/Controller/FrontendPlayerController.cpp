// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/FrontendPlayerController.h"

#include "FrontendDebugHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"

void AFrontendPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	// 查找场景中所有带 "Default" Tag 的 CameraActor
	TArray<AActor*> foundCameras;
	UGameplayStatics::GetAllActorsOfClassWithTag(this, ACameraActor::StaticClass(), FName("Default"),foundCameras );
	if (!foundCameras.IsEmpty())
	{
		// 将第一个找到的摄像机设为当前视角目标
		SetViewTarget(foundCameras[0]);
	}
}
