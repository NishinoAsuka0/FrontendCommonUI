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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPushSoftWidgetDelegate, UWidget_ActivatableBase*,PushedWidget);

UCLASS()
class FRONTENDUI_API UAsyncAction_PushSoftWidget : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, meta=(WorldContext = "WorldContextObject", HidePin = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Push Soft Widget To Widget Stack"))
	static UAsyncAction_PushSoftWidget* PushSoftWidget(const UObject* WorldContextObject, 
	APlayerController* PlayerController, 
	TSoftClassPtr<UWidget_ActivatableBase> InSoftWidgetClass, 
	UPARAM(meta = (Categories = "Frontend.WidgetStack"))FGameplayTag InWidgetStackTag, 
	bool bFocusOnNewlyPushedWidget);
	
	//Begin UBlueprintAsyncActionBase
	virtual void Activate() override;
	//end
	
	UPROPERTY(BlueprintAssignable)
	FOnPushSoftWidgetDelegate OnWidgetCreatedBeforePush;
	
	UPROPERTY(BlueprintAssignable)
	FOnPushSoftWidgetDelegate AfterPush;
private:
	TWeakObjectPtr<UWorld> CachedOwningWorld;
	TWeakObjectPtr<APlayerController> CachedOwningPC;
	TSoftClassPtr<UWidget_ActivatableBase> CachedWidgetClass;
	FGameplayTag CachedWidgetTag;
	bool bCachedFocusOnNewlyPushedWidget = true;
};
