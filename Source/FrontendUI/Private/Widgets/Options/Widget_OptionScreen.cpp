// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Options/Widget_OptionScreen.h"

#include "FrontendDebugHelper.h"
#include "ICommonInputModule.h"
#include "Input/CommonUIInputTypes.h"
#include "AsyncAction/AsyncAction_PushSoftWidget.h"

void UWidget_OptionScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	if (!ResetAction.IsNull())
	{
		ResetActionHandle = RegisterUIActionBinding(
			FBindUIActionArgs(
				ResetAction,
				true,
				FSimpleDelegate::CreateUObject(this, &ThisClass::OnResetBoundActionTriggered)
			)
		);
	}
	RegisterUIActionBinding(
	FBindUIActionArgs(
		ICommonInputModule::GetSettings().GetDefaultBackAction(),
		true,
		FSimpleDelegate::CreateUObject(this, &ThisClass::OnBackBoundActionTriggered)
	)
	);
	TabListWidget_OptionsTabs->OnTabSelected.AddUniqueDynamic(this, &ThisClass::OnOptionsTabSelected);
}

void UWidget_OptionScreen::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	for (UListDataObject_Collection*TabCollection : GetOrCreateDataRegistry()->GetRegisteredOptionsTabCollections())
	{
		if (!TabCollection)	continue;
		const FName TabID = TabCollection->GetDataID();
		if (TabListWidget_OptionsTabs->GetTabButtonBaseByID(TabID) != nullptr)
		{
			//已经创建过了
			continue;
		}
		TabListWidget_OptionsTabs->RequestRegisterTab(TabID, TabCollection->GetDataDisplayName());
	}
}

UOptionsDataRegistry* UWidget_OptionScreen::GetOrCreateDataRegistry()
{
	if (!OwningDataRegistry)
	{
		//不存在才创建
		OwningDataRegistry = NewObject<UOptionsDataRegistry>();
		OwningDataRegistry->InitOptionsDataRegistry(GetOwningLocalPlayer());
	}
	if (OwningDataRegistry)	return OwningDataRegistry;
	
	DebugHelper::Print(TEXT("UWidget_OptionScreen::GetOrCreateDataRegistry() OwningDataRegistry Is NULL"));
	return nullptr;
}

void UWidget_OptionScreen::OnResetBoundActionTriggered()
{
	DebugHelper::Print(TEXT("UWidget_OptionScreen::OnResetBoundActionTriggered()"));
}

void UWidget_OptionScreen::OnBackBoundActionTriggered()
{
	DeactivateWidget();
}

void UWidget_OptionScreen::OnOptionsTabSelected(FName TabID)
{
	//获取tab对应的子tab数据
	TArray<UListDataObject_Base*> FoundListSourceItem = GetOrCreateDataRegistry()->GetListSourceItemByTabID(TabID);
	
	CommonListView_OptionsTabs->SetListItems(FoundListSourceItem);
	CommonListView_OptionsTabs->RequestRefresh();
	
	if ((CommonListView_OptionsTabs->GetNumItems() != 0))
	{
		CommonListView_OptionsTabs->NavigateToIndex(0);
		CommonListView_OptionsTabs->SetSelectedIndex(0);
	}
}
