// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Options/OptionsDataInteractionHelper.h"

FOptionsDataInteractionHelper::FOptionsDataInteractionHelper(const FString& InSetterOrGetterFuncPath)
{
	// 缓存 GameUserSettings 单例的弱引用
	CachedWeakGameUserSettings = UFrontendGameUserSettings::Get();
	// 缓存属性路径，避免每次读写时重复解析
	CachedDynamicFunctionPath = InSetterOrGetterFuncPath;
}

FString FOptionsDataInteractionHelper::GetValueAsString() const
{
	FString OutStringValue;
	// 通过属性路径从 GameUserSettings 读取值
	PropertyPathHelpers::GetPropertyValueAsString(CachedWeakGameUserSettings.Get(), CachedDynamicFunctionPath, OutStringValue);
	return OutStringValue;
}

void FOptionsDataInteractionHelper::SetValueFromString(const FString& InString)
{
	// 通过属性路径向 GameUserSettings 写入值
	PropertyPathHelpers::SetPropertyValueFromString(CachedWeakGameUserSettings.Get(), CachedDynamicFunctionPath, InString);
}
