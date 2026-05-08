// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Options/DataObjects/ListDataObject_String.h"

#include "FrontendDebugHelper.h"


void UListDataObject_String::OnDataObjectInitialized()
{
	// ① 默认选中第一个选项
	if (!AvailableOptionsStringArray.IsEmpty())
	{
		CurrentStringValue = AvailableOptionsStringArray[0];
	}

	// ② 有自定义默认值则覆盖
	if (HasDefaultValues())
	{
		CurrentStringValue = GetDefaultValueAsString();
	}

	// ③ 有 DynamicGetter 则读取已保存的值（最高优先级）
	if (DataDynamicGetter)
	{
		if (!DataDynamicGetter->GetValueAsString().IsEmpty())
		{
			CurrentStringValue = DataDynamicGetter->GetValueAsString();
		}
	}

	// 根据内部值设置显示文本
	if (!TrySetDisplayTextFromStringValue(CurrentStringValue))
	{
		CurrentTextValue = FText::FromString(TEXT("Invalid Options"));
	}

}

void UListDataObject_String::AddDynamicOption(const FString& InStringValue, const FText& InDisplayText)
{
	AvailableOptionsStringArray.Add(InStringValue);
	AvailableOptionsTextArray.Add(InDisplayText);
}

void UListDataObject_String::AdvanceToNextOption()
{
	if (!AvailableOptionsStringArray.IsEmpty() && !AvailableOptionsTextArray.IsEmpty())
	{
		// 取模实现循环切换
		const int32 CurrentFoundIndex = AvailableOptionsStringArray.IndexOfByKey(CurrentStringValue);
		int32 nextStringIndex = (CurrentFoundIndex + 1) % AvailableOptionsStringArray.Num();
		CurrentStringValue = AvailableOptionsStringArray[nextStringIndex];
		if (!TrySetDisplayTextFromStringValue(CurrentStringValue))
		{
			CurrentTextValue = FText::FromString(TEXT("Invalid Options"));
		}
	}
	// 写回 GameUserSettings
	if (DataDynamicSetter)
	{
		DataDynamicSetter->SetValueFromString(CurrentStringValue);
	}
	// 通知 UI 层更新
	NotifyListDataModified(this);
}

void UListDataObject_String::AdvanceToPreviousOption()
{
	if (!AvailableOptionsStringArray.IsEmpty() && !AvailableOptionsTextArray.IsEmpty())
	{
		// 取模实现循环切换（防负数的安全取模）
		const int32 CurrentFoundIndex = AvailableOptionsStringArray.IndexOfByKey(CurrentStringValue);
		int32 preStringIndex = (CurrentFoundIndex + AvailableOptionsStringArray.Num() - 1) % AvailableOptionsStringArray.Num();
		CurrentStringValue = AvailableOptionsStringArray[preStringIndex];
		if (!TrySetDisplayTextFromStringValue(CurrentStringValue))
		{
			CurrentTextValue = FText::FromString(TEXT("Invalid Options"));
		}
	}

	if (DataDynamicSetter)
	{
		DataDynamicSetter->SetValueFromString(CurrentStringValue);
	}
	NotifyListDataModified(this);
}

void UListDataObject_String::OnRotatorInitiatedValueChange(const FText& InNewText)
{
	// 根据显示文本查找对应的内部值索引
	int32 Index = AvailableOptionsTextArray.IndexOfByPredicate(
	[InNewText](const FText& InText)
	{
		return InText.EqualTo(InNewText);
	}
	);

	if ((Index != INDEX_NONE && AvailableOptionsStringArray.IsValidIndex(Index)))
	{
		CurrentStringValue = AvailableOptionsStringArray[Index];
		CurrentTextValue = InNewText;

		if (DataDynamicSetter)
		{
			DataDynamicSetter->SetValueFromString(CurrentStringValue);
		}
		NotifyListDataModified(this);
	}
}

bool UListDataObject_String::CanResetBackToDefault() const
{
	return HasDefaultValues() && CurrentStringValue != GetDefaultValueAsString();
}

bool UListDataObject_String::TryResetBackToDefault()
{
	if (CanResetBackToDefault())
	{
		CurrentStringValue = GetDefaultValueAsString();
		TrySetDisplayTextFromStringValue(CurrentStringValue);
		if (DataDynamicSetter)
		{
			DataDynamicSetter->SetValueFromString(CurrentStringValue);
		}
		NotifyListDataModified(this, EOptionsListDataModifyReason::ResetTotDefault);
		return true;
	}
	return false;
}

bool UListDataObject_String::TrySetDisplayTextFromStringValue(const FString& InStringValue)
{
	// 根据内部值查找对应的显示文本
	const int32 CurrentFoundIndex = AvailableOptionsStringArray.Find(InStringValue);
	if (AvailableOptionsTextArray.IsValidIndex(CurrentFoundIndex))
	{
		CurrentTextValue = AvailableOptionsTextArray[CurrentFoundIndex];
		return true;
	}

	return false;
}
