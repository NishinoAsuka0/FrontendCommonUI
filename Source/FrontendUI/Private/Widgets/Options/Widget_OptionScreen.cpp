// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Options/Widget_OptionScreen.h"

#include "FrontendDebugHelper.h"
#include "ICommonInputModule.h"
#include "Input/CommonUIInputTypes.h"
#include "AsyncAction/AsyncAction_PushSoftWidget.h"
#include "FrontendSettings/FrontendGameUserSettings.h"
#include "Subsystems/FrontendUISubSystem.h"

void UWidget_OptionScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 注册 Reset UIAction
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
	// 注册 Back UIAction（使用 CommonInput 默认返回键）
	RegisterUIActionBinding(
	FBindUIActionArgs(
		ICommonInputModule::GetSettings().GetDefaultBackAction(),
		true,
		FSimpleDelegate::CreateUObject(this, &ThisClass::OnBackBoundActionTriggered)
	)
	);
	// 绑定 Tab 选中事件
	TabListWidget_OptionsTabs->OnTabSelected.AddUniqueDynamic(this, &ThisClass::OnOptionsTabSelected);

	// 绑定 ListView 悬停和选中事件
	CommonListView_OptionsTabs->OnItemIsHoveredChanged().AddUObject(this, &ThisClass::OnListViewItemHovered);
	CommonListView_OptionsTabs->OnItemSelectionChanged().AddUObject(this, &ThisClass::OnListViewItemSelected);
}

void UWidget_OptionScreen::NativeOnActivated()
{
	Super::NativeOnActivated();

	// 遍历所有 Tab 分组，注册尚未创建的 Tab 按钮
	for (UListDataObject_Collection*TabCollection : GetOrCreateDataRegistry()->GetRegisteredOptionsTabCollections())
	{
		if (!TabCollection)	continue;
		const FName TabID = TabCollection->GetDataID();
		// 已创建则跳过
		if (TabListWidget_OptionsTabs->GetTabButtonBaseByID(TabID) != nullptr)
		{
			continue;
		}
		TabListWidget_OptionsTabs->RequestRegisterTab(TabID, TabCollection->GetDataDisplayName());
	}
}

UOptionsDataRegistry* UWidget_OptionScreen::GetOrCreateDataRegistry()
{
	if (!OwningDataRegistry)
	{
		// 懒创建：首次访问时新建并初始化
		OwningDataRegistry = NewObject<UOptionsDataRegistry>();
		OwningDataRegistry->InitOptionsDataRegistry(GetOwningLocalPlayer());
	}
	if (OwningDataRegistry)	return OwningDataRegistry;

	DebugHelper::Print(TEXT("UWidget_OptionScreen::GetOrCreateDataRegistry() OwningDataRegistry Is NULL"));
	return nullptr;
}

void UWidget_OptionScreen::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	// 释放 DataRegistry 内存
	OwningDataRegistry = nullptr;
	// 确保设置持久化到 ini
	UFrontendGameUserSettings::Get()->ApplySettings(true);
}

UWidget* UWidget_OptionScreen::NativeGetDesiredFocusTarget() const
{
	// 返回当前选中的列表项控件作为焦点目标
	if (UObject* SelectedObject = CommonListView_OptionsTabs->GetSelectedItem())
	{
		if (UUserWidget* SelectedEntryWidget = CommonListView_OptionsTabs->GetEntryWidgetFromItem(SelectedObject))
		{
			return SelectedEntryWidget;
		}
	}
	return Super::NativeGetDesiredFocusTarget();
}

void UWidget_OptionScreen::OnResetBoundActionTriggered()
{
	if (ResettableDataArray.IsEmpty())
	{
		return;
	}

	// 获取当前激活 Tab 的名称用于确认弹窗提示
	UCommonButtonBase* SelectedButton = TabListWidget_OptionsTabs->GetTabButtonBaseByID(TabListWidget_OptionsTabs->GetActiveTab());
	FString SelectedButtonName = CastChecked<UFrontendCommonButtonBase>(SelectedButton)->GetButtonDisplayText().ToString();

	// 弹出确认弹窗，用户确认后才执行重置
	UFrontendUISubSystem::Get(this)->PushConfirmScreenToModalStackAsync(
		EConfirmScreenType::YesNo,
		FText::FromString(TEXT("Reset")),
		FText::FromString(TEXT("Are you sure you want to reset all the settings under the ") + SelectedButtonName + TEXT(" tab ?")),
		[this](EConfirmScreenButtonType ClickedButtonType)
		{
			if (ClickedButtonType != EConfirmScreenButtonType::Confirm)
			{
				return;
			}

			bool bHasDataFailedToReset = false;
			// 设置标志防止 OnListViewItemValueChanged 重复处理
			bIsResettingDataToDefault = true;
			for (UListDataObject_Base* DataToReset: ResettableDataArray)
			{
				if (!DataToReset)
				{
					continue;
				}
				if (DataToReset->TryResetBackToDefault())
				{
					DebugHelper::Print(DataToReset->GetDataDisplayName().ToString() + TEXT("was reset "));
				}
				else
				{
					bHasDataFailedToReset = true;
					DebugHelper::Print(DataToReset->GetDataDisplayName().ToString() + TEXT("failed to reset "));
				}
			}
			// 全部重置成功则清空列表并禁用 Reset Action
			if (!bHasDataFailedToReset)
			{
				ResettableDataArray.Empty();
				RemoveActionBinding(ResetActionHandle);
			}
			bIsResettingDataToDefault = false;
		}
	);
}

void UWidget_OptionScreen::OnBackBoundActionTriggered()
{
	DeactivateWidget();
}

void UWidget_OptionScreen::OnOptionsTabSelected(FName TabID)
{
	DetailView_ListEntryInfo->ClearDetailsViewInfo();

	// 获取 tab 对应的子数据列表并设置到 ListView
	TArray<UListDataObject_Base*> FoundListSourceItems = GetOrCreateDataRegistry()->GetListSourceItemByTabID(TabID);

	CommonListView_OptionsTabs->SetListItems(FoundListSourceItems);
	CommonListView_OptionsTabs->RequestRefresh();

	// 默认选中第一项
	if ((CommonListView_OptionsTabs->GetNumItems() != 0))
	{
		CommonListView_OptionsTabs->NavigateToIndex(0);
		CommonListView_OptionsTabs->SetSelectedIndex(0);
	}

	// 重建 ResettableDataArray：遍历当前 Tab 的数据项
	ResettableDataArray.Empty();
	for (UListDataObject_Base* FoundListSourceItem : FoundListSourceItems)
	{
		if (FoundListSourceItem)
		{
			// 绑定数据修改委托（避免重复绑定）
			if (!FoundListSourceItem->OnListDataModifiedDelegate.IsBoundToObject(this))
			{
				FoundListSourceItem->OnListDataModifiedDelegate.AddUObject(this, &ThisClass::OnListViewItemValueChanged);
			}
			// 将可重置的项加入列表
			if (FoundListSourceItem->CanResetBackToDefault())
			{
				ResettableDataArray.AddUnique(FoundListSourceItem);
			}
		}
	}
	// 根据是否有可重置项来启用/禁用 Reset Action
	if (ResettableDataArray.IsEmpty())
	{
		RemoveActionBinding(ResetActionHandle);
	}
	else
	{
		if (!GetActionBindings().Contains(ResetActionHandle))
		{
			AddActionBinding(ResetActionHandle);
		}
	}
}

void UWidget_OptionScreen::OnListViewItemHovered(UObject* HoveredItem, bool IsHovered)
{
	if (HoveredItem)
	{
		// 通知列表项控件悬停状态变更
		UWidget_ListEntry_Base* HoveredEntryWidget = CommonListView_OptionsTabs->GetEntryWidgetFromItem<UWidget_ListEntry_Base>(HoveredItem);
		if (HoveredEntryWidget)
		{
			HoveredEntryWidget->NativeOnListEntryWidgetHovered(IsHovered);
		}
		// 悬停时更新 DetailView
		if (IsHovered)
		{
			DetailView_ListEntryInfo->UpdateDetailsViewInfo(CastChecked<UListDataObject_Base>(HoveredItem), TryGetEntryWidgetClassName(HoveredItem));
		}
		else
		{
			// 取消悬停时恢复到选中项的详情
			UListDataObject_Base* SelectedItem = CommonListView_OptionsTabs->GetSelectedItem<UListDataObject_Base>();
			if (SelectedItem)
			{
				DetailView_ListEntryInfo->UpdateDetailsViewInfo(CastChecked<UListDataObject_Base>(SelectedItem), TryGetEntryWidgetClassName(SelectedItem));
			}
		}
	}
}

void UWidget_OptionScreen::OnListViewItemSelected(UObject* SelectedItem)
{
	if (SelectedItem)
	{
		DetailView_ListEntryInfo->UpdateDetailsViewInfo(CastChecked<UListDataObject_Base>(SelectedItem), TryGetEntryWidgetClassName(SelectedItem));
	}
}

void UWidget_OptionScreen::OnListViewItemValueChanged(UListDataObject_Base* SelectedItem,
	EOptionsListDataModifyReason ChangedReason)
{
	if (!SelectedItem)
	{
		return;
	}
	// 批量重置中跳过单个修改的处理
	if (bIsResettingDataToDefault) return;

	// 动态管理 ResettableDataArray：与默认值不同则加入，相同则移除
	if (SelectedItem->CanResetBackToDefault())
	{
		ResettableDataArray.AddUnique(SelectedItem);
		if (!GetActionBindings().Contains(ResetActionHandle))
		{
			AddActionBinding(ResetActionHandle);
		}
	}
	else
	{
		if (ResettableDataArray.Contains(SelectedItem))
		{
			ResettableDataArray.Remove(SelectedItem);
		}
	}
	// 无可重置项时禁用 Reset Action
	if (ResettableDataArray.IsEmpty())
	{
		RemoveActionBinding(ResetActionHandle);
	}
}

FString UWidget_OptionScreen::TryGetEntryWidgetClassName(UObject* InOwningListItem)
{
	// 获取列表项对应的控件类名，用于 DetailView 调试信息显示
	if (UUserWidget* FoundEntryWidget = CommonListView_OptionsTabs->GetEntryWidgetFromItem(InOwningListItem))
	{
		return FoundEntryWidget->GetClass()->GetName();
	}
	return TEXT("EntryWidget Not Valid");
}
