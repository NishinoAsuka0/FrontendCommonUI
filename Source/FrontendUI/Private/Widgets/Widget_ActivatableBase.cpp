// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Widget_ActivatableBase.h"

AFrontendPlayerController* UWidget_ActivatableBase::GetOwningFrontendPlayerController()
{
	if (!CachedOwningFrontendPlayerController.IsValid())
	{
		CachedOwningFrontendPlayerController = GetOwningPlayer<AFrontendPlayerController>();
	}
	return CachedOwningFrontendPlayerController.IsValid()?CachedOwningFrontendPlayerController.Get():nullptr;
}
