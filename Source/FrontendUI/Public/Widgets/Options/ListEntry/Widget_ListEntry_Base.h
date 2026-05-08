// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "CommonTextBlock.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Widgets/Options/DataObjects/ListDataObject_Base.h"
#include "Widget_ListEntry_Base.generated.h"

/**
 * 列表项基类
 * 实现 IUserObjectListEntry 接口，绑定数据修改委托，管理悬停事件
 */
UCLASS(Abstract, BlueprintType, meta = (DisableNativeTick))
class FRONTENDUI_API UWidget_ListEntry_Base : public UCommonUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
public:
	/** 蓝图可实现事件：列表项悬停状态变更 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On List Entry Widget Hovered"))
	void BP_OnListEntryWidgetHovered(bool bWasHovered, bool bIsEntryWidgetStillSelected);
	/** 通知蓝图悬停事件 */
	void NativeOnListEntryWidgetHovered(bool bWasHovered);

protected:
	/** 蓝图可实现事件：手柄模式下返回需要聚焦的子控件 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Get Widget To Focus For Gamepad"))
	UWidget* BP_GetWidgetToFocusForGamepad() const;

	// ---- IUserObjectListEntry 接口 ----
	/** 列表项数据对象赋值时触发：设置可见性并调用 OnOwningListDataObjectSet */
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	/** 列表项释放时触发 */
	virtual void NativeOnEntryReleased() override;

	// ---- UUserWidget 接口 ----
	/** 获得焦点时对于手柄输入自动转发到指定子控件 */
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;

	/** 数据对象赋值时设置显示名称并绑定修改委托 */
	virtual void OnOwningListDataObjectSet(UListDataObject_Base* InOwningListDataObject);

	/** 数据对象被修改时由委托触发，子类重写以刷新 UI */
	virtual void OnOwningListDataObjectModified(UListDataObject_Base* InOwningListDataObject, EOptionsListDataModifyReason Reason);

	/** 选中当前列表项 */
	void SelectThisWidget();

private:
	// ---- 绑定的控件 ----
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	UCommonTextBlock * CommonText_SettingDisplayName;
};
