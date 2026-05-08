// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Options/Widget_OptionsDetailView.h"

void UWidget_OptionsDetailView::UpdateDetailsViewInfo(UListDataObject_Base* ListDataObject,
	const FString& InEntryWidgetClassName)
{
	if (!ListDataObject)
	{
		return;
	}
	// 设置标题
	CommonTextBlock_Title->SetText(ListDataObject->GetDataDisplayName());
	// 设置描述图片（有则显示，无则隐藏）
	if (!ListDataObject->GetSoftDescriptionImage().IsNull())
	{
		CommonLazyImage_DescriptionImage->SetBrushFromLazyTexture(ListDataObject->GetSoftDescriptionImage());
		CommonLazyImage_DescriptionImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		CommonLazyImage_DescriptionImage->SetVisibility(ESlateVisibility::Collapsed);
	}
	// 设置描述文本
	CommonRichTextBlock_Description->SetText(ListDataObject->GetDescriptionText());
	// 动态信息：显示数据对象类名和控件类名（调试用）
	const FString DynamicDetails = FString::Printf(
		TEXT("Data Object Class: <Bold> %s </>\n\nEntry Widget Class: <Bold>%s</>"),
		*ListDataObject->GetClass()->GetName(),
		*InEntryWidgetClassName
	);
	CommonRichTextBlock_DynamicDetails->SetText(FText::FromString(DynamicDetails));
	// 设置禁用原因
	CommonRichTextBlock_DisabledReason->SetText(ListDataObject->GetDisableRichText());

}

void UWidget_OptionsDetailView::ClearDetailsViewInfo()
{
	CommonTextBlock_Title->SetText(FText::GetEmpty());
	CommonLazyImage_DescriptionImage->SetVisibility(ESlateVisibility::Collapsed);
	CommonRichTextBlock_Description->SetText(FText::GetEmpty());
	CommonRichTextBlock_DisabledReason->SetText(FText::GetEmpty());
	CommonRichTextBlock_DynamicDetails->SetText(FText::GetEmpty());
}

void UWidget_OptionsDetailView::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	ClearDetailsViewInfo();
}
