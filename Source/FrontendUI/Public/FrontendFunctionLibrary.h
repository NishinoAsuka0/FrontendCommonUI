// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Widgets/Widget_ActivatableBase.h"
#include "FrontendFunctionLibrary.generated.h"

/**
 * 前端功能库
 * 提供蓝图可调用的静态工具方法，通过 GameplayTag 查询 DeveloperSettings 中配置的软引用资源
 */
UCLASS()
class FRONTENDUI_API UFrontendFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/** 通过 Widget GameplayTag 从 DeveloperSettings 中查找对应的控件软引用类 */
	UFUNCTION(BlueprintPure, Category = "Frontend Function Library")
	static TSoftClassPtr<UWidget_ActivatableBase> GetFrontendSoftWidgetClassByTag(UPARAM(meta = (Categories = "Frontend.Widget")) FGameplayTag InWidgetTag);

	/** 通过 Image GameplayTag 从 DeveloperSettings 中查找对应的图片软引用 */
	UFUNCTION(BlueprintPure, Category = "Frontend Function Library")
	static TSoftObjectPtr<UTexture2D> GetOptionsSoftImageByTag(UPARAM(meta = (Categories = "Frontend.Image")) FGameplayTag InImageTag);
};
