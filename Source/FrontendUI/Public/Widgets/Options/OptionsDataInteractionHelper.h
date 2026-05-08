// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PropertyPathHelpers.h"
#include "FrontendSettings/FrontendGameUserSettings.h"

/**
 * 选项数据交互辅助类
 * 通过 PropertyPathHelpers 读写 GameUserSettings 的属性值，实现数据对象与配置的解耦
 */
class FRONTENDUI_API FOptionsDataInteractionHelper
{
public:
	/** 初始化：缓存属性路径和 GameUserSettings 弱引用 */
	FOptionsDataInteractionHelper(const FString& InSetterOrGetterFuncPath);

	/** 从 GameUserSettings 读取属性值字符串 */
	FString GetValueAsString() const;

	/** 将字符串写入 GameUserSettings 的属性 */
	void SetValueFromString(const FString& InString);
private:
	/** 缓存的属性路径，避免每次重复解析 */
	FCachedPropertyPath CachedDynamicFunctionPath;

	/** GameUserSettings 的弱引用，不阻止 GC */
	TWeakObjectPtr<UFrontendGameUserSettings> CachedWeakGameUserSettings;
};
