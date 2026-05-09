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
		// 通过 WorldContext 获取 World，再获取 GameInstance 上的 Subsystem
		UWorld* World = GEngine->GetWorldFromContextObject(UWorldContextObject, EGetWorldErrorMode::Assert);
		return UGameInstance::GetSubsystem<UFrontendUISubSystem>(World->GetGameInstance());
	}
	return nullptr;
}

bool UFrontendUISubSystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// 仅非 DedicatedServer 实例创建 UI 子系统，服务器不需要 UI
	if (!CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance())
	{
		// 检查是否有派生类，只返回基类实例
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
}

void UFrontendUISubSystem::PushSoftWidgetToStackAsync(const FGameplayTag& tag,
TSoftClassPtr<UWidget_ActivatableBase> InSoftWidgetClass, TFunction<void(EAsyncPushWidgetState, UWidget_ActivatableBase*)> Callback)
{
	check(!InSoftWidgetClass.IsNull());

	// 异步加载控件类，避免同步加载造成卡顿
	UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
		InSoftWidgetClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateLambda(
			[InSoftWidgetClass, this, tag, Callback]()
			{
				UClass* LoadClass = InSoftWidgetClass.Get();
				check(LoadClass);

				if (!IsValid(CreatedPrimaryLayoutWidget))
				{
					UE_LOG(LogTemp, Error, TEXT("[FrontendUISubSystem] PrimaryLayout not registered, cannot push widget."));
					return;
				}

				// 按 GameplayTag 查找对应的 WidgetStack
				UCommonActivatableWidgetContainerBase* foundWidgetStack = CreatedPrimaryLayoutWidget->FindWidgetStackByTag(tag);

				// 阶段一：控件创建后、推入前，通过回调让调用方初始化控件数据
				UWidget_ActivatableBase* CreatedWidget = foundWidgetStack->AddWidget<UWidget_ActivatableBase>(
					LoadClass,
					[Callback](UWidget_ActivatableBase& widget)
					{
						Callback(EAsyncPushWidgetState::OnCreatedBefore, &widget);
					}
				);

				// 阶段二：控件已推入 Stack
				Callback(EAsyncPushWidgetState::AfterPush, CreatedWidget);
			}
		)
		);
}

void UFrontendUISubSystem::PushConfirmScreenToModalStackAsync(EConfirmScreenType InScrennType,
	const FText& InScreenTitle, const FText& InScreenMsg, TFunction<void(EConfirmScreenButtonType)> Callback)
{
	// 根据弹窗类型创建对应的信息对象
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
		// 通过通用推送接口将确认弹窗推入 Modal Stack
		PushSoftWidgetToStackAsync(
			FrontendGameplayTags::Frontend_WidgetStack_Modal,
			UFrontendFunctionLibrary::GetFrontendSoftWidgetClassByTag(FrontendGameplayTags::Frontend_WidgetStack_ConfirmScreen),
			[CreatedInfoObject, Callback](EAsyncPushWidgetState InPushState, UWidget_ActivatableBase* InWidget)
			{
				// 控件创建后立即初始化弹窗内容
				if (InPushState == EAsyncPushWidgetState::OnCreatedBefore)
				{
					UWidget_ConfirmScreen* CreatedConfirmScreen = CastChecked<UWidget_ConfirmScreen>(InWidget);
					CreatedConfirmScreen->InitConfirmScreen(CreatedInfoObject, Callback);
				}
			}
			);
	}
}
