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
			// 创建节点并缓存弹窗配置参数
			UUAsyncAction_PushConfirmScreen*Node = NewObject<UUAsyncAction_PushConfirmScreen>();
			Node->CachedOwningWorld = World;
			Node->ScreenType = ScreenType;
			Node->InScreenTitle = InScreenTitle;
			Node->InScreenDescription = InScreenDescription;

			// 注册到 GameInstance，确保异步操作随 GameInstance 生命周期
			Node->RegisterWithGameInstance(World);
			return Node;
		}
	}
	return nullptr;
}

void UUAsyncAction_PushConfirmScreen::Activate()
{
	// 委托 Subsystem 推送确认弹窗，用户点击按钮后广播结果
	UFrontendUISubSystem::Get(CachedOwningWorld.Get())->PushConfirmScreenToModalStackAsync(ScreenType, InScreenTitle, InScreenDescription,
		[this](EConfirmScreenButtonType ButtonType)
		{
			OnClickedButton.Broadcast(ButtonType);
			SetReadyToDestroy();
		}
	);
}
