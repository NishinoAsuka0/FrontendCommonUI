// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Components/FrontendCommonButtonBase.h"

#include "Subsystems/FrontendUISubSystem.h"

void UFrontendCommonButtonBase::SetButtonText(FText InText)
{
	if (CommonTextBlock_ButtonText && !InText.IsEmpty())
	{
		CommonTextBlock_ButtonText->SetText(bUserUpperCaseForButtonText?InText.ToUpper():InText);
	}
}

void UFrontendCommonButtonBase::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();
	
	if (CommonTextBlock_ButtonText && GetCurrentTextStyleClass())
	{
		CommonTextBlock_ButtonText->SetStyle(GetCurrentTextStyleClass());
	}
}

void UFrontendCommonButtonBase::NativeOnHovered()
{
	Super::NativeOnHovered();
	
	if (!ButtonDescriptionText.IsEmpty())
	{
		UFrontendUISubSystem::Get(this)->OnButtonTextUpdatedDelegate.Broadcast(this, ButtonDescriptionText);
	}
}

void UFrontendCommonButtonBase::NativeOnUnhovered()
{
	Super::NativeOnUnhovered();
	UFrontendUISubSystem::Get(this)->OnButtonTextUpdatedDelegate.Broadcast(this, FText::GetEmpty());
}

void UFrontendCommonButtonBase::NativePreConstruct()
{
	Super::NativePreConstruct();
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
