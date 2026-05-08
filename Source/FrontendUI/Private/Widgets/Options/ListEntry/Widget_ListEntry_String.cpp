// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Options/ListEntry/Widget_ListEntry_String.h"

#include "CommonInputSubsystem.h"


void UWidget_ListEntry_String::OnOwningListDataObjectSet(UListDataObject_Base* InOwningListDataObject)
{
	Super::OnOwningListDataObjectSet(InOwningListDataObject);

	// 缓存强类型引用
	CachedOwningStringDataObject = CastChecked<UListDataObject_String>(InOwningListDataObject);

	// 填充 Rotator 选项并设置当前选中项
	CommonRotator_AvailableOptions->PopulateTextLabels(CachedOwningStringDataObject->GetDisplayText());
	CommonRotator_AvailableOptions->SetSelectedOptionByText(CachedOwningStringDataObject->GetCurDisplayText());
}

void UWidget_ListEntry_String::OnOwningListDataObjectModified(UListDataObject_Base* InOwningListDataObject,
	EOptionsListDataModifyReason Reason)
{
	// 数据变更时更新 Rotator 显示文本
	if (CachedOwningStringDataObject)
	{
		CommonRotator_AvailableOptions->SetSelectedOptionByText(CachedOwningStringDataObject->GetCurDisplayText());
	}
}

void UWidget_ListEntry_String::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 绑定增减按钮点击事件
	CommonButton_Decrease->OnClicked().AddUObject(this, &ThisClass::OnDecreaseButtonClicked);
	CommonButton_Increase->OnClicked().AddUObject(this, &ThisClass::OnIncreaseButtonClicked);

	// 点击 Rotator 时选中当前列表项；旋转时触发值变更
	CommonRotator_AvailableOptions->OnClicked().AddLambda([this](){SelectThisWidget();});
	CommonRotator_AvailableOptions->OnRotatedEvent.AddUObject(this, &ThisClass::OnRotatorValueChanged);
}

void UWidget_ListEntry_String::OnDecreaseButtonClicked()
{
	if (CachedOwningStringDataObject)
	{
		CachedOwningStringDataObject->AdvanceToPreviousOption();
	}
	SelectThisWidget();
}

void UWidget_ListEntry_String::OnIncreaseButtonClicked()
{
	if (CachedOwningStringDataObject)
	{
		CachedOwningStringDataObject->AdvanceToNextOption();
	}
	SelectThisWidget();
}

void UWidget_ListEntry_String::OnRotatorValueChanged(int32 Value, bool bUserInitialized)
{
	if (!CachedOwningStringDataObject)
	{
		return;
	}

	// 仅手柄模式且为用户主动触发时更新数据对象
	UCommonInputSubsystem* CommonInputSubsystem = GetInputSubsystem();
	if (!CommonInputSubsystem || !bUserInitialized)
	{
		return;
	}

	if (CommonInputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad)
	{
		CachedOwningStringDataObject->OnRotatorInitiatedValueChange(CommonRotator_AvailableOptions->GetSelectedText());
	}
}
