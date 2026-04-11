// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OptionsDataRegistry.h"
#include "Widgets/Widget_ActivatableBase.h"
#include "Widgets/Components/FrontendCommonListView.h"
#include "Widgets/Components/FrontendTabListWidgetBase.h"
#include "Widget_OptionScreen.generated.h"

/**
 * 
 */
UCLASS(Abstract, BlueprintType, meta = (DisableNativeTick))
class FRONTENDUI_API UWidget_OptionScreen : public UWidget_ActivatableBase
{
	GENERATED_BODY()
protected:
	//begin UUserWidget Interface
	virtual void NativeOnInitialized() override;
	//end
	
	//begin UCommonActivatableBase
	virtual void NativeOnActivated() override;
	
private:
	UOptionsDataRegistry* GetOrCreateDataRegistry();
	
	void OnResetBoundActionTriggered();
	void OnBackBoundActionTriggered();
	
	UFUNCTION()
	void OnOptionsTabSelected(FName TabID);
	/*Bound Widget*/
	UPROPERTY(meta = (BindWidget))
	UFrontendTabListWidgetBase* TabListWidget_OptionsTabs;
	
	UPROPERTY(meta = (BindWidget))
	UFrontendCommonListView* CommonListView_OptionsTabs;
	
	UPROPERTY(Transient)
	UOptionsDataRegistry* OwningDataRegistry;
	
	UPROPERTY(EditDefaultsOnly, Category = "Frontend Options Screen", meta = (RowType = "/Script/CommonUI.CommonInputActionDataBase"))
	FDataTableRowHandle ResetAction;
	
	FUIActionBindingHandle ResetActionHandle;
};
