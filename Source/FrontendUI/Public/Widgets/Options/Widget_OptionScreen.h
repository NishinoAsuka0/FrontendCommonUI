// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OptionsDataRegistry.h"
#include "Widget_OptionsDetailView.h"
#include "Widgets/Widget_ActivatableBase.h"
#include "Widgets/Components/FrontendCommonListView.h"
#include "Widgets/Components/FrontendTabListWidgetBase.h"
#include "Widget_OptionScreen.generated.h"

/**
 * 选项界面主控件
 * 管理 DataRegistry 生命周期、Tab 切换、ListView 交互和 Reset 功能
 */
UCLASS(Abstract, BlueprintType, meta = (DisableNativeTick))
class FRONTENDUI_API UWidget_OptionScreen : public UWidget_ActivatableBase
{
	GENERATED_BODY()
protected:
	// ---- UUserWidget Interface ----
	/** 初始化时注册 UIAction（Reset、Back）和绑定 Tab/List 事件 */
	virtual void NativeOnInitialized() override;
	/** 停用时清理 DataRegistry + ApplySettings 保存 */
	virtual void NativeOnDeactivated() override;
	/** 返回当前选中列表项作为焦点目标 */
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	// ---- UCommonActivatableBase ----
	/** 激活时创建/复用 DataRegistry 并注册 Tab 按钮 */
	virtual void NativeOnActivated() override;

private:
	/** 懒创建 DataRegistry，首次访问时初始化 */
	UOptionsDataRegistry* GetOrCreateDataRegistry();

	/** Reset 操作：弹出确认弹窗后批量重置可重置的选项 */
	void OnResetBoundActionTriggered();
	/** Back 操作：退出 Options 界面 */
	void OnBackBoundActionTriggered();

	/** Tab 切换时更新 ListView 数据源和绑定修改委托 */
	UFUNCTION()
	void OnOptionsTabSelected(FName TabID);

	/** 悬停项变更时更新 DetailView */
	void OnListViewItemHovered(UObject* HoveredItem, bool IsHovered);
	/** 选中项变更时更新 DetailView */
	void OnListViewItemSelected(UObject* SelectedItem);

	/** 选项值变更时管理 ResettableDataArray 和 ResetAction 的启用/禁用 */
	void OnListViewItemValueChanged(UListDataObject_Base* SelectedItem, EOptionsListDataModifyReason ChangedReason);

	/** 获取列表项对应的控件类名（供 DetailView 调试信息） */
	FString TryGetEntryWidgetClassName(UObject* InOwningListItem);

	// ---- 绑定的控件 ----
	UPROPERTY(meta = (BindWidget))
	UFrontendTabListWidgetBase* TabListWidget_OptionsTabs;

	UPROPERTY(meta = (BindWidget))
	UFrontendCommonListView* CommonListView_OptionsTabs;

	UPROPERTY(meta = (BindWidget))
	UWidget_OptionsDetailView* DetailView_ListEntryInfo;

	/** 当前持有的数据注册表 */
	UPROPERTY(Transient)
	UOptionsDataRegistry* OwningDataRegistry;

	/** Reset 操作的输入配置 */
	UPROPERTY(EditDefaultsOnly, Category = "Frontend Options Screen", meta = (RowType = "/Script/CommonUI.CommonInputActionDataBase"))
	FDataTableRowHandle ResetAction;

	/** Reset Action 的绑定句柄，用于动态启用/禁用 */
	FUIActionBindingHandle ResetActionHandle;

	/** 当前存在非默认值的可重置选项列表 */
	UPROPERTY(Transient)
	TArray<UListDataObject_Base*>ResettableDataArray;

	/** 批量重置中标志：跳过 OnListViewItemValueChanged 的处理 */
	bool bIsResettingDataToDefault = false;
};
