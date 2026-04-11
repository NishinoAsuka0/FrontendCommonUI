// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncAction/AsyncAction_PushSoftWidget.h"

#include "Subsystems/FrontendUISubSystem.h"

UAsyncAction_PushSoftWidget* UAsyncAction_PushSoftWidget::PushSoftWidget(const UObject* WorldContextObject,
                                                                         APlayerController* PlayerController, TSoftClassPtr<UWidget_ActivatableBase> InSoftWidgetClass,
                                                                         UPARAM(meta = (Categories = "Frontend.WidgetStack"))FGameplayTag InWidgetStackTag, bool bFocusOnNewlyPushedWidget)
{
	checkf(!InSoftWidgetClass.IsNull(), TEXT("PushSoftWidgetToStack Error In BluePrint"));
	
	if (GEngine)
	{
		if (UWorld* world = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			UAsyncAction_PushSoftWidget*Node = NewObject<UAsyncAction_PushSoftWidget>();
			Node->CachedOwningWorld = world;
			Node->CachedWidgetClass = InSoftWidgetClass;
			Node->CachedOwningPC = PlayerController;
			Node->CachedWidgetTag = InWidgetStackTag;
			Node->bCachedFocusOnNewlyPushedWidget = bFocusOnNewlyPushedWidget;
			
			Node->RegisterWithGameInstance(world);
			return Node;
		}
	}
	return  nullptr;
}

void UAsyncAction_PushSoftWidget::Activate()
{
	Super::Activate();
	
	UFrontendUISubSystem* FrontendUISubSystem = UFrontendUISubSystem::Get(CachedOwningWorld.Get());
	
	FrontendUISubSystem->PushSoftWidgetToStackAsync(CachedWidgetTag, CachedWidgetClass, 
		[this](EAsyncPushWidgetState InPushSTATE, UWidget_ActivatableBase* InWidget)
		{
			switch (InPushSTATE)
			{
			case EAsyncPushWidgetState::OnCreatedBefore:
				InWidget->SetOwningPlayer(CachedOwningPC.Get());
				OnWidgetCreatedBeforePush.Broadcast(InWidget);
				break;
			case EAsyncPushWidgetState::AfterPush:
				AfterPush.Broadcast(InWidget);
				if (bCachedFocusOnNewlyPushedWidget)
				{
					if (UWidget* widgetToFocus = InWidget->GetDesiredFocusTarget())
					{
						widgetToFocus->SetFocus();
					}
					
					SetReadyToDestroy();
				}
				break;
			default:
					break;
			}
		}	
	);
}
