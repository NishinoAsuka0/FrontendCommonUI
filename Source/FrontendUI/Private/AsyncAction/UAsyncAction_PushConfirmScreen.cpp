// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncAction/UAsyncAction_PushConfirmScreen.h"

#include "Subsystems/FrontendUISubSystem.h"

UUAsyncAction_PushConfirmScreen* UUAsyncAction_PushConfirmScreen::PushConfirmScreen(const UObject* WorldContextObject,
                                                                                    EConfirmScreenType ScreenType, FText InScreenTitle, FText InScreenDescription)
{
	if (GEngine)
	{
		if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			UUAsyncAction_PushConfirmScreen*Node = NewObject<UUAsyncAction_PushConfirmScreen>();
			Node->CachedOwningWorld = World;
			Node->ScreenType = ScreenType;
			Node->InScreenTitle = InScreenTitle;
			Node->InScreenDescription = InScreenDescription;
			
			Node->RegisterWithGameInstance(World);
			return Node;
		}
	}
	return nullptr;
}

void UUAsyncAction_PushConfirmScreen::Activate()
{
	UFrontendUISubSystem::Get(CachedOwningWorld.Get())->PushConfirmScreenToModalStackAsync(ScreenType, InScreenTitle, InScreenDescription,
		[this](EConfirmScreenButtonType ButtonType)
		{
			OnClickedButton.Broadcast(ButtonType);
			SetReadyToDestroy();
		}
	);
}
