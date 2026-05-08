// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Components/FrontendCommonButtonBase.h"
#include "Widgets/Components/FrontendCommonRotator.h"
#include "Widgets/Options/DataObjects/ListDataObject_String.h"
#include "Widgets/Options/ListEntry/Widget_ListEntry_Base.h"
#include "Widget_ListEntry_String.generated.h"

/**
 * 字符串列表项控件
 * 绑定 Rotator 显示选项文本，通过增减按钮切换选项值
 */
UCLASS(Abstract, BlueprintType, meta = (DisableNativeTick))
class FRONTENDUI_API UWidget_ListEntry_String : public UWidget_ListEntry_Base
{
	GENERATED_BODY()
protected:
	// ---- UWidget_ListEntry_Base 接口 ----
	/** Cast 为 String 数据对象，填充 Rotator 选项并设置当前值 */
	virtual void OnOwningListDataObjectSet(UListDataObject_Base* InOwningListDataObject) override;

	/** 数据变更时更新 Rotator 显示 */
	virtual void OnOwningListDataObjectModified(UListDataObject_Base* InOwningListDataObject, EOptionsListDataModifyReason Reason) override;

	// ---- UUserWidget ----
	/** 绑定增减按钮点击和 Rotator 旋转事件 */
	virtual void NativeOnInitialized() override;
private:
	/** 减少按钮：调用 AdvanceToPreviousOption */
	void OnDecreaseButtonClicked();
	/** 增加按钮：调用 AdvanceToNextOption */
	void OnIncreaseButtonClicked();
	/** Rotator 值变更：手柄模式下更新数据对象 */
	void OnRotatorValueChanged(int32 Value, bool bUserInitialized);

	// ---- 绑定的控件 ----
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	UFrontendCommonButtonBase* CommonButton_Decrease;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	UFrontendCommonRotator* CommonRotator_AvailableOptions;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	UFrontendCommonButtonBase* CommonButton_Increase;

	/** 缓存强类型数据对象引用 */
	UPROPERTY(Transient)
	UListDataObject_String* CachedOwningStringDataObject;
};
