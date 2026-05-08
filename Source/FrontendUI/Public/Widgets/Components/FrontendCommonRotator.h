// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonRotator.h"
#include "FrontendCommonRotator.generated.h"

/**
 * 前端通用旋转选择器
 * 扩展 UCommonRotator，支持通过显示文本定位并选中对应选项
 */
UCLASS(Abstract, BlueprintType, meta = (DisableNativeTick))
class FRONTENDUI_API UFrontendCommonRotator : public UCommonRotator
{
	GENERATED_BODY()
public:
	/** 根据显示文本查找匹配选项并设为当前选中；找不到匹配时直接设置文本 */
	void SetSelectedOptionByText(const FText& InTextOption);
};
