// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Options/DataObjects/ListDataObject_Value.h"
#include "ListDataObject_String.generated.h"

/**
 * 字符串类型数据对象
 * 用双数组（String + Text）存储选项的内部值和显示文本，支持循环切换
 */
UCLASS()
class FRONTENDUI_API UListDataObject_String : public UListDataObject_Value
{
	GENERATED_BODY()
public:
	/** 添加选项：内部值 + 显示文本 */
	void AddDynamicOption(const FString& InStringValue, const FText& InDisplayText);
	/** 切换到下一个选项（循环），通过 Setter 写回 GameUserSettings */
	void AdvanceToNextOption();
	/** 切换到上一个选项（循环），通过 Setter 写回 GameUserSettings */
	void AdvanceToPreviousOption();
	/** Rotator 控件触发值变更时调用 */
	void OnRotatorInitiatedValueChange(const FText& InNewText);

	FORCEINLINE const TArray<FText> GetDisplayText() {return AvailableOptionsTextArray; }
	FORCEINLINE FText GetCurDisplayText() {return CurrentTextValue; }

protected:
	/** 当前内部值 */
	FString CurrentStringValue;
	/** 当前显示文本 */
	FText CurrentTextValue;
	/** 所有可用的内部值 */
	TArray<FString> AvailableOptionsStringArray;
	/** 所有可用的显示文本 */
	TArray<FText> AvailableOptionsTextArray;

	// ---- UListDataObject_Base 接口 ----
	/** 三级初始化优先级：硬编码默认 → 自定义默认 → 已保存的值 */
	virtual void OnDataObjectInitialized() override;
	/** 当前值 != 默认值时可重置 */
	virtual bool CanResetBackToDefault() const override;
	/** 重置为默认值并通过 Setter 写回 */
	virtual bool TryResetBackToDefault() override;

	/** 根据内部值查找对应的显示文本 */
	bool TrySetDisplayTextFromStringValue(const FString& InStringValue);
};
