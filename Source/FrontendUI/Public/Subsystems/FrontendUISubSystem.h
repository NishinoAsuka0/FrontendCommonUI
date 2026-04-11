// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Widgets/Widget_ActivatableBase.h"
#include "GameplayTagContainer.h" 
#include "FrontendTypes/FrontendEnumTypes.h"
#include "Widgets/Components/FrontendCommonButtonBase.h"
#include "FrontendUISubSystem.generated.h"

/**
 * 
 */
class UWidget_PrimaryLayout;

enum class EAsyncPushWidgetState : uint8
{
	OnCreatedBefore,
	AfterPush
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnButtonTextUpdatedDelegate, UFrontendCommonButtonBase*, BroadcastingButton, FText, DescriptionText);


UCLASS()
class FRONTENDUI_API UFrontendUISubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	static UFrontendUISubSystem* Get(const UObject* UWorldContextObject);
	
	//from USubSystem
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//end 
	
	UFUNCTION(BlueprintCallable)
	void RegisterCreatedPrimaryLayoutWidget(UWidget_PrimaryLayout* InCreatedWidget);
	
	void PushSoftWidgetToStackAsync(const FGameplayTag& tag, TSoftClassPtr<UWidget_ActivatableBase> InSoftWidgetClass, TFunction<void(EAsyncPushWidgetState, UWidget_ActivatableBase*)> Callback);

	void PushConfirmScreenToModalStackAsync(EConfirmScreenType InScrennType, const FText& InScreenTitle, const FText& InScreenMsg, TFunction<void(EConfirmScreenButtonType)> Callback);
	
	UPROPERTY(BlueprintAssignable)
	FOnButtonTextUpdatedDelegate OnButtonTextUpdatedDelegate;

private:
	UPROPERTY(Transient)
	UWidget_PrimaryLayout* CreatedPrimaryLayoutWidget = nullptr;
};
