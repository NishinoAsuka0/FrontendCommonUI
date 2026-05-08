// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Options/OptionsDataInteractionHelper.h"
#include "Widgets/Options/DataObjects/ListDataObject_Base.h"
#include "ListDataObject_Value.generated.h"

/**
 * 值类型数据对象（抽象）
 * 持有 Getter/Setter 交互辅助类和默认值，提供读写 GameUserSettings 属性值的通道
 */
UCLASS(Abstract)
class FRONTENDUI_API UListDataObject_Value : public UListDataObject_Base
{
	GENERATED_BODY()
public:
	/** 设置动态取值器（通过 PropertyPath 从 GameUserSettings 读取） */
	void SetDataDynamicGetter(const TSharedPtr<FOptionsDataInteractionHelper>& InDynamicGetter);
	/** 设置动态设值器（通过 PropertyPath 向 GameUserSettings 写入） */
	void SetDataDynamicSetter(const TSharedPtr<FOptionsDataInteractionHelper>& InDynamicSetter);

	/** 设置默认值字符串 */
	void SetDefaultValueFromString(const FString& InDefaultValue){DefaultStringValue = InDefaultValue;};

	/** 检查 DefaultStringValue 是否已设置 */
	virtual bool HasDefaultValues() const override{return DefaultStringValue.IsSet();};

protected:
	/** 获取默认值字符串 */
	FString GetDefaultValueAsString() const {return DefaultStringValue.GetValue();};

	/** 取值辅助类 */
	TSharedPtr<FOptionsDataInteractionHelper> DataDynamicGetter;
	/** 设值辅助类 */
	TSharedPtr<FOptionsDataInteractionHelper> DataDynamicSetter;

private:
	/** TOptional 区分"未设置"和"默认值为空字符串" */
	TOptional<FString> DefaultStringValue;
};
