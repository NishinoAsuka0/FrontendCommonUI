// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Components/FrontendCommonRotator.h"

#include "CommonTextBlock.h"

void UFrontendCommonRotator::SetSelectedOptionByText(const FText& InTextOption)
{
	// 在 Rotator 的文本标签列表中查找匹配项
	const int32 FoundIndex = TextLabels.IndexOfByPredicate(
		[InTextOption](const FText& TextItem)->bool
		{
			return TextItem.EqualTo(InTextOption);
		}
	);
	if (FoundIndex != INDEX_NONE)
	{
		// 找到匹配：选中对应索引的选项
		SetSelectedItem(FoundIndex);
	}
	else
	{
		// 找不到匹配：降级处理，直接显示原始文本
		MyText->SetText(InTextOption);
	}
}
