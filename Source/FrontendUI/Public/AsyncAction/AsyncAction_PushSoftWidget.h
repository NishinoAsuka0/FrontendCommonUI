// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Widgets/Widget_ActivatableBase.h"
#include "AsyncAction_PushSoftWidget.generated.h"

/**
 *
 */

/** 推送控件完成委托：广播被推送的控件实例 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPushSoftWidgetDelegate, UWidget_ActivatableBase*,PushedWidget);

/**
 * 异步推送软引用控件到 WidgetStack
 * 支持蓝图 Latent 节点，提供两阶段回调：创建后（OnWidgetCreatedBeforePush）和推入后（AfterPush）
 */
UCLASS()
class FRONTENDUI_API UAsyncAction_PushSoftWidget : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	/** 创建并缓存推送参数，注册到 GameInstance */
	UFUNCTION(BlueprintCallable, meta=(WorldContext = "WorldContextObject", HidePin = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Push Soft Widget To Widget Stack"))
	static UAsyncAction_PushSoftWidget* PushSoftWidget(const UObject* WorldContextObject,
	APlayerController* PlayerController,
	TSoftClassPtr<UWidget_ActivatableBase> InSoftWidgetClass,
	UPARAM(meta = (Categories = "Frontend.WidgetStack"))FGameplayTag InWidgetStackTag,
	bool bFocusOnNewlyPushedWidget);

	/** 激活时调用 Subsystem 的异步推送 */
	virtual void Activate() override;

	/** 控件已创建但尚未推入 Stack 时广播 */
	UPROPERTY(BlueprintAssignable)
	FOnPushSoftWidgetDelegate OnWidgetCreatedBeforePush;

	/** 控件已推入 Stack 后广播 */
	UPROPERTY(BlueprintAssignable)
	FOnPushSoftWidgetDelegate AfterPush;
private:
	TWeakObjectPtr<UWorld> CachedOwningWorld;
	TWeakObjectPtr<APlayerController> CachedOwningPC;
	TSoftClassPtr<UWidget_ActivatableBase> CachedWidgetClass;
	FGameplayTag CachedWidgetTag;
	bool bCachedFocusOnNewlyPushedWidget = true;
};
