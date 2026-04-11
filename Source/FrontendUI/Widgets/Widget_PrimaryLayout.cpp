// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget_PrimaryLayout.h"
#include "FrontendDebugHelper.h"

UCommonActivatableWidgetContainerBase* UWidget_PrimaryLayout::FindWidgetStackByTag(const FGameplayTag& tag)
{
	checkf(RegisteredWidgetStackMap.Contains(tag), TEXT("Can n ot find the widget stack by the tag %s"), *tag.ToString());
	return RegisteredWidgetStackMap.FindRef(tag);
}

void UWidget_PrimaryLayout::RegisterWidgetStack(UPARAM(meta = (Categories = "Frontend.WidgetStack")) FGameplayTag InStackTag,
                                                UCommonActivatableWidgetContainerBase* registerWidget)
{
	if (!IsDesignTime())
	{
		if (!RegisteredWidgetStackMap.Contains(InStackTag))
		{
			RegisteredWidgetStackMap.Add(InStackTag, registerWidget);
			/*DebugHelper::Print(TEXT("Widget Stack Registered Tag : " + InStackTag.ToString()));*/
		}
	}
}
