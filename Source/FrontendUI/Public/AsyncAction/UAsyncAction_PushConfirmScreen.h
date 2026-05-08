// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FrontendTypes/FrontendEnumTypes.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "UAsyncAction_PushConfirmScreen.generated.h"

/**
 *
 */

/** 确认弹窗按钮点击委托：广播用户点击的按钮类型 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FConfirmScreenButtonClickedDelegate, EConfirmScreenButtonType, ClickedButtonType);

/**
 * 异步推送确认弹窗到 Modal Stack
 * 封装弹窗创建细节，调用方只需关注用户点击了哪个按钮
 */
UCLASS()
class FRONTENDUI_API UUAsyncAction_PushConfirmScreen : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	/** 创建节点并缓存弹窗参数，注册到 GameInstance */
	UFUNCTION(BlueprintCallable, meta=(WorldContext = "WorldContextObject", HidePin = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Show Confirmation Screen"))
	static UUAsyncAction_PushConfirmScreen* PushConfirmScreen(
			const UObject* WorldContextObject,
			EConfirmScreenType ScreenType,
			FText InScreenTitle,
			FText InScreenDescription
	);

	/** 激活时调用 Subsystem 推送确认弹窗 */
	virtual void Activate() override;

	/** 用户点击按钮后广播按钮类型 */
	UPROPERTY(BlueprintAssignable)
	FConfirmScreenButtonClickedDelegate OnClickedButton;
private:
	TWeakObjectPtr<UWorld> CachedOwningWorld;
	EConfirmScreenType ScreenType;
	FText InScreenTitle;
	FText InScreenDescription;
};
