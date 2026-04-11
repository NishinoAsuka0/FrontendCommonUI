// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/FrontendUISubSystem.h"
#include "FrontendDebugHelper.h"
#include "FrontendFunctionLibrary.h"
#include "FrontendGamePlayTags.h"
#include "Engine/AssetManager.h"
#include "FrontendUI/Widgets/Widget_PrimaryLayout.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "Widgets/Widget_ConfirmScreen.h"

UFrontendUISubSystem* UFrontendUISubSystem::Get(const UObject* UWorldContextObject)
{
	if (GEngine)
	{
		UWorld* World = GEngine->GetWorldFromContextObject(UWorldContextObject, EGetWorldErrorMode::Assert);
		return UGameInstance::GetSubsystem<UFrontendUISubSystem>(World->GetGameInstance());
	}
	return nullptr;
}

bool UFrontendUISubSystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance())
	{
		DebugHelper::Print(TEXT("Widget ShouldCreateSubsystem Checker"));
		TArray<UClass*>foundClasses;
		GetDerivedClasses(GetClass(), foundClasses);
		return foundClasses.IsEmpty();
	}
	return false;
}

void UFrontendUISubSystem::RegisterCreatedPrimaryLayoutWidget(UWidget_PrimaryLayout* InCreatedWidget)
{
	check(InCreatedWidget);
	CreatedPrimaryLayoutWidget = InCreatedWidget;
	
	DebugHelper::Print(TEXT("Widget created widget created"));
}

void UFrontendUISubSystem::PushSoftWidgetToStackAsync(const FGameplayTag& tag,
TSoftClassPtr<UWidget_ActivatableBase> InSoftWidgetClass, TFunction<void(EAsyncPushWidgetState, UWidget_ActivatableBase*)> Callback)
{
	check(!InSoftWidgetClass.IsNull());
	
	UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
		InSoftWidgetClass.ToSoftObjectPath(), 
		FStreamableDelegate::CreateLambda(
			[InSoftWidgetClass, this, tag, Callback]()
			{
				UClass* LoadClass = InSoftWidgetClass.Get();
				check(LoadClass);
				UCommonActivatableWidgetContainerBase* foundWidgetStack = CreatedPrimaryLayoutWidget->FindWidgetStackByTag(tag);
				
				UWidget_ActivatableBase* CreatedWidget = foundWidgetStack->AddWidget<UWidget_ActivatableBase>(
					LoadClass,
					[Callback](UWidget_ActivatableBase& widget)
					{
						Callback(EAsyncPushWidgetState::OnCreatedBefore, &widget);
					}
				);
				
				Callback(EAsyncPushWidgetState::AfterPush, CreatedWidget);
			}
		)
		);
}

void UFrontendUISubSystem::PushConfirmScreenToModalStackAsync(EConfirmScreenType InScrennType,
	const FText& InScreenTitle, const FText& InScreenMsg, TFunction<void(EConfirmScreenButtonType)> Callback)
{
	UConfirmScreenInfoObject* CreatedInfoObject = nullptr;
	switch (InScrennType)
	{
	case EConfirmScreenType::Ok:
		CreatedInfoObject = UConfirmScreenInfoObject::CreateOKScreen(InScreenTitle, InScreenMsg);
		break;
	case EConfirmScreenType::YesNo:
		CreatedInfoObject = UConfirmScreenInfoObject::CreateYesNoScreen(InScreenTitle, InScreenMsg);
		break;
	case EConfirmScreenType::OKCancel:
		CreatedInfoObject = UConfirmScreenInfoObject::CreateOKCancelScreen(InScreenTitle, InScreenMsg);
		break;
	case EConfirmScreenType::Unknown:
		break;
	}
	if (CreatedInfoObject)
	{
		PushSoftWidgetToStackAsync(
			FrontendGameplayTags::Frontend_WidgetStack_Modal,
			UFrontendFunctionLibrary::GetFrontendSoftWidgetClassByTag(FrontendGameplayTags::Frontend_WidgetStack_ConfirmScreen),
			[CreatedInfoObject, Callback](EAsyncPushWidgetState InPushState, UWidget_ActivatableBase* InWidget)
			{
				if (InPushState == EAsyncPushWidgetState::OnCreatedBefore)
				{
					UWidget_ConfirmScreen* CreatedConfirmScreen = CastChecked<UWidget_ConfirmScreen>(InWidget);
					CreatedConfirmScreen->InitConfirmScreen(CreatedInfoObject, Callback);
				}
			}
			);
	}
}
