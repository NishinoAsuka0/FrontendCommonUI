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
			// 创建节点并缓存所有推送参数
			UAsyncAction_PushSoftWidget*Node = NewObject<UAsyncAction_PushSoftWidget>();
			Node->CachedOwningWorld = world;
			Node->CachedWidgetClass = InSoftWidgetClass;
			Node->CachedOwningPC = PlayerController;
			Node->CachedWidgetTag = InWidgetStackTag;
			Node->bCachedFocusOnNewlyPushedWidget = bFocusOnNewlyPushedWidget;

			// 注册到 GameInstance，使异步操作随 GameInstance 生命周期
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

	// 委托 Subsystem 执行异步加载和推送，通过回调处理两个阶段
	FrontendUISubSystem->PushSoftWidgetToStackAsync(CachedWidgetTag, CachedWidgetClass,
		[this](EAsyncPushWidgetState InPushSTATE, UWidget_ActivatableBase* InWidget)
		{
			switch (InPushSTATE)
			{
			case EAsyncPushWidgetState::OnCreatedBefore:
				// 阶段一：设置 PC 并广播，允许蓝图在推入前初始化控件
				InWidget->SetOwningPlayer(CachedOwningPC.Get());
				OnWidgetCreatedBeforePush.Broadcast(InWidget);
				break;
			case EAsyncPushWidgetState::AfterPush:
				// 阶段二：广播推入完成，可选择聚焦新控件
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
