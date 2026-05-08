// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget_PrimaryLayout.h"
#include "FrontendDebugHelper.h"

UCommonActivatableWidgetContainerBase* UWidget_PrimaryLayout::FindWidgetStackByTag(const FGameplayTag& tag)
{
	// 找不到对应 Stack 时 crash，确保 Tag 已正确注册
	checkf(RegisteredWidgetStackMap.Contains(tag), TEXT("Can n ot find the widget stack by the tag %s"), *tag.ToString());
	return RegisteredWidgetStackMap.FindRef(tag);
}

void UWidget_PrimaryLayout::RegisterWidgetStack(UPARAM(meta = (Categories = "Frontend.WidgetStack")) FGameplayTag InStackTag,
                                                UCommonActivatableWidgetContainerBase* registerWidget)
{
	// 仅在非设计时注册，避免编辑器预览时干扰
	if (!IsDesignTime())
	{
		// 重复注册同一 Tag 会被忽略
		if (!RegisteredWidgetStackMap.Contains(InStackTag))
		{
			RegisteredWidgetStackMap.Add(InStackTag, registerWidget);
		}
	}
}
