// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Components/FrontendCommonButtonBase.h"

#include "Subsystems/FrontendUISubSystem.h"

void UFrontendCommonButtonBase::SetButtonText(FText InText)
{
	if (CommonTextBlock_ButtonText && !InText.IsEmpty())
	{
		// 根据配置决定是否转为大写
		CommonTextBlock_ButtonText->SetText(bUserUpperCaseForButtonText?InText.ToUpper():InText);
	}
}

FText UFrontendCommonButtonBase::GetButtonDisplayText()
{
	if (CommonTextBlock_ButtonText)
	{
		return CommonTextBlock_ButtonText->GetText();
	}
	return FText::GetEmpty();
}

void UFrontendCommonButtonBase::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();

	// 样式变更时同步到文本控件
	if (CommonTextBlock_ButtonText && GetCurrentTextStyleClass())
	{
		CommonTextBlock_ButtonText->SetStyle(GetCurrentTextStyleClass());
	}
}

void UFrontendCommonButtonBase::NativeOnHovered()
{
	Super::NativeOnHovered();

	// 悬停时广播描述文本，供底部提示栏显示
	if (!ButtonDescriptionText.IsEmpty())
	{
		UFrontendUISubSystem::Get(this)->OnButtonTextUpdatedDelegate.Broadcast(this, ButtonDescriptionText);
	}
}

void UFrontendCommonButtonBase::NativeOnUnhovered()
{
	Super::NativeOnUnhovered();
	// 取消悬停时广播空文本，清除提示栏
	UFrontendUISubSystem::Get(this)->OnButtonTextUpdatedDelegate.Broadcast(this, FText::GetEmpty());
}

void UFrontendCommonButtonBase::NativePreConstruct()
{
	Super::NativePreConstruct();
	// 预构建时设置初始文本
	SetButtonText(ButtonDisplayText);

	if (ButtonStyleType)
	{
		SetStyle(ButtonStyleType);
	}
}

void UFrontendCommonButtonBase::NativeDestruct()
{
	Super::NativeDestruct();
}
