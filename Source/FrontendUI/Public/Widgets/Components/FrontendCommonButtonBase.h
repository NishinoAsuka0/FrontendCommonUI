// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "FrontendCommonButtonBase.generated.h"

/**
 * 前端通用按钮基类
 * 支持设置显示文本、大写转换、按钮描述文本广播（悬停时通知全局提示栏）
 */
UCLASS(Abstract, BlueprintType, meta = (DisableNativeTick))
class FRONTENDUI_API UFrontendCommonButtonBase : public UCommonButtonBase
{
	GENERATED_BODY()
public:
	/** 设置按钮文本，支持大写转换 */
	UFUNCTION(BlueprintCallable)
	void SetButtonText(FText InText);

	/** 获取当前按钮显示的文本 */
	UFUNCTION(BlueprintCallable)
	FText GetButtonDisplayText();
private:
	/** 文本样式变更时同步更新内部文本控件 */
	virtual void NativeOnCurrentTextStyleChanged() override;
	/** 悬停时通过 Subsystem 广播按钮描述文本（供底部提示栏使用） */
	virtual void NativeOnHovered() override;
	/** 取消悬停时广播空文本 */
	virtual void NativeOnUnhovered() override;
	/** 预构建时设置初始文本和样式 */
	virtual void NativePreConstruct() override;
	virtual void NativeDestruct() override;

	// ---- 绑定的控件 ----
	UPROPERTY(meta = (BindWidgetOptional))
	UCommonTextBlock* CommonTextBlock_ButtonText;

	/** 按钮显示文本 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fronend Button", meta = (AllowPrivateAccess = true))
	FText ButtonDisplayText;

	/** 是否将按钮文本转为大写 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fronend Button", meta = (AllowPrivateAccess = true))
	bool bUserUpperCaseForButtonText = false;

	/** 悬停时广播的描述文本（显示在底部提示栏） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fronend Button", meta = (AllowPrivateAccess = true))
	FText ButtonDescriptionText;

	/** 按钮样式类 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fronend Button", meta = (AllowPrivateAccess = true))
	TSubclassOf<UCommonButtonStyle> ButtonStyleType;
};
