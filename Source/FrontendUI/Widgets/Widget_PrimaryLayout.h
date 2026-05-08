// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "Widget_PrimaryLayout.generated.h"

/**
 * 根布局控件
 * 注册和管理多个 WidgetStack，按 GameplayTag 索引，为所有 UI 界面提供容器
 */
class UCommonActivatableWidgetContainerBase;

UCLASS(Abstract, BlueprintType, meta = (DisableNavieTick))
class FRONTENDUI_API UWidget_PrimaryLayout : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/** 按 GameplayTag 查找已注册的 WidgetStack，找不到则 crash */
	UCommonActivatableWidgetContainerBase* FindWidgetStackByTag(const FGameplayTag& tag);
protected:
	/** 蓝图调用：注册 WidgetStack 到 Tag→Stack 映射 */
	UFUNCTION(BlueprintCallable)
	void RegisterWidgetStack(UPARAM(meta = (Categories = "Frontend.WidgetStack")) FGameplayTag InStackTag, UCommonActivatableWidgetContainerBase* registerWidget);
private:
	/** Tag → WidgetStack 映射表，设计时不注册 */
	UPROPERTY(Transient)
	TMap<FGameplayTag, UCommonActivatableWidgetContainerBase*> RegisteredWidgetStackMap;
};
