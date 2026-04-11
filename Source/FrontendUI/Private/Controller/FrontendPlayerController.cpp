// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/FrontendPlayerController.h"

#include "FrontendDebugHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"

void AFrontendPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);
	
	TArray<AActor*> foundCameras;
	UGameplayStatics::GetAllActorsOfClassWithTag(this, ACameraActor::StaticClass(), FName("Default"),foundCameras );
	DebugHelper::Print(TEXT(" FoundCameras: ") + FString::FromInt(foundCameras.Num() ));
	if (!foundCameras.IsEmpty())
	{
		SetViewTarget(foundCameras[0]);
	}
}
