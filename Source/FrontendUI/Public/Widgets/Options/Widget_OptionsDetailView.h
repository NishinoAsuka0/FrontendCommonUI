// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonTextBlock.h"
#include "CommonLazyImage.h"
#include "CommonRichTextBlock.h"
#include "Blueprint/UserWidget.h"
#include "DataObjects/ListDataObject_Base.h"
#include "Widget_OptionsDetailView.generated.h"

/**
 * 选项详情面板
 * 显示当前选中选项的标题、描述图片、描述文本、动态信息和禁用原因
 */
UCLASS(Abstract, BlueprintType, meta = (DisableNativeTick))
class FRONTENDUI_API UWidget_OptionsDetailView : public UUserWidget
{
	GENERATED_BODY()
public:
	/** 更新详情面板：填充标题、图片、描述、动态信息和禁用原因 */
	void UpdateDetailsViewInfo(UListDataObject_Base* ListDataObject, const FString& InEntryWidgetClassName = FString());
	/** 清空所有详情面板内容 */
	void ClearDetailsViewInfo();

protected:
	/** 初始化时清空面板 */
	virtual void NativeOnInitialized() override;
private:
	UPROPERTY(meta = (BindWidget))
	UCommonTextBlock* CommonTextBlock_Title;

	UPROPERTY(meta = (BindWidget))
	UCommonLazyImage* CommonLazyImage_DescriptionImage;

	UPROPERTY(meta = (BindWidget))
	UCommonRichTextBlock* CommonRichTextBlock_Description;

	UPROPERTY(meta = (BindWidget))
	UCommonRichTextBlock* CommonRichTextBlock_DynamicDetails;

	UPROPERTY(meta = (BindWidget))
	UCommonRichTextBlock* CommonRichTextBlock_DisabledReason;

};
