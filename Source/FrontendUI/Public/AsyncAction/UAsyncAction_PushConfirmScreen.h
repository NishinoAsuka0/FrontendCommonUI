// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FrontendTypes/FrontendEnumTypes.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "UAsyncAction_PushConfirmScreen.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FConfirmScreenButtonClickedDelegate, EConfirmScreenButtonType, ClickedButtonType);

UCLASS()
class FRONTENDUI_API UUAsyncAction_PushConfirmScreen : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, meta=(WorldContext = "WorldContextObject", HidePin = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Show Confirmation Screen"))
	static UUAsyncAction_PushConfirmScreen* PushConfirmScreen(
			const UObject* WorldContextObject,
			EConfirmScreenType ScreenType,
			FText InScreenTitle,
			FText InScreenDescription
	);
	
	//Begin UBlueprintAsyncActionBase
	virtual void Activate() override;
	//end
	
	UPROPERTY(BlueprintAssignable)
	FConfirmScreenButtonClickedDelegate OnClickedButton;
private:
	TWeakObjectPtr<UWorld> CachedOwningWorld;
	EConfirmScreenType ScreenType;
	FText InScreenTitle;
	FText InScreenDescription;
};
