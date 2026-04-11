// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Widget_ConfirmScreen.h"

#include "FrontendDebugHelper.h"
#include "ICommonInputModule.h"
#include "Widgets/Components/FrontendCommonButtonBase.h"

UConfirmScreenInfoObject* UConfirmScreenInfoObject::CreateOKScreen(const FText& InScreenTitle,
                                                                   const FText& InScreenMessage)
{
	UConfirmScreenInfoObject* Result = NewObject<UConfirmScreenInfoObject>();
	Result->ScreenTitle = InScreenTitle;
	Result->ScreenMessage = InScreenMessage;
	
	FConfirmScreenButtonInfo OKButtonInfo;
	OKButtonInfo.ButtonType = EConfirmScreenButtonType::Closed;
	OKButtonInfo.ButtonTextToDisplay = FText::FromString(TEXT("OK"));
	
	Result->ConfirmScreenButtons.Add(OKButtonInfo);
	return Result;
}

UConfirmScreenInfoObject* UConfirmScreenInfoObject::CreateYesNoScreen(const FText& InScreenTitle,
	const FText& InScreenMessage)
{
	UConfirmScreenInfoObject* Result = NewObject<UConfirmScreenInfoObject>();
	Result->ScreenTitle = InScreenTitle;
	Result->ScreenMessage = InScreenMessage;
	
	FConfirmScreenButtonInfo YesButtonInfo;
	YesButtonInfo.ButtonType = EConfirmScreenButtonType::Confirm;
	YesButtonInfo.ButtonTextToDisplay = FText::FromString(TEXT("Yes"));
	
	FConfirmScreenButtonInfo NoButtonInfo;
	NoButtonInfo.ButtonType = EConfirmScreenButtonType::Cancelled;
	NoButtonInfo.ButtonTextToDisplay = FText::FromString(TEXT("No"));
	
	Result->ConfirmScreenButtons.Add(YesButtonInfo);
	Result->ConfirmScreenButtons.Add(NoButtonInfo);
	return Result;
}

UConfirmScreenInfoObject* UConfirmScreenInfoObject::CreateOKCancelScreen(const FText& InScreenTitle,
                                                                         const FText& InScreenMessage)
{
	UConfirmScreenInfoObject* Result = NewObject<UConfirmScreenInfoObject>();
	Result->ScreenTitle = InScreenTitle;
	Result->ScreenMessage = InScreenMessage;
	
	FConfirmScreenButtonInfo OKButtonInfo;
	OKButtonInfo.ButtonType = EConfirmScreenButtonType::Confirm;
	OKButtonInfo.ButtonTextToDisplay = FText::FromString(TEXT("OK"));
	
	FConfirmScreenButtonInfo CancelButtonInfo;
	CancelButtonInfo.ButtonType = EConfirmScreenButtonType::Cancelled;
	CancelButtonInfo.ButtonTextToDisplay = FText::FromString(TEXT("Cancel"));
	
	Result->ConfirmScreenButtons.Add(OKButtonInfo);
	Result->ConfirmScreenButtons.Add(CancelButtonInfo);
	return Result;
}

void UWidget_ConfirmScreen::InitConfirmScreen(UConfirmScreenInfoObject* InScreenInfoObject,
	TFunction<void(EConfirmScreenButtonType)> InConfirmScreenCallback)
{
	if (InScreenInfoObject && CommonTextBlock_Title && CommonTextBlock_Message && DynamicEntryBox_Buttons)
	{
		CommonTextBlock_Title->SetText(InScreenInfoObject->ScreenTitle);
		CommonTextBlock_Message->SetText(InScreenInfoObject->ScreenMessage);
		
		if (DynamicEntryBox_Buttons->GetNumEntries() != 0)
		{
			DynamicEntryBox_Buttons->Reset<UFrontendCommonButtonBase>(
				[](UFrontendCommonButtonBase& Button)
				{
					Button.OnClicked().Clear();
				}	
			);
		}
		if (!InScreenInfoObject->ConfirmScreenButtons.IsEmpty())
		{
			for (const FConfirmScreenButtonInfo& ButtonInfo : InScreenInfoObject->ConfirmScreenButtons)
			{
				FDataTableRowHandle inputActionRowHandle;
				switch (ButtonInfo.ButtonType)
				{
				case EConfirmScreenButtonType::Confirm:
						inputActionRowHandle = ICommonInputModule::GetSettings().GetDefaultClickAction();
					case EConfirmScreenButtonType::Cancelled:
					inputActionRowHandle = ICommonInputModule::GetSettings().GetDefaultBackAction();
					break;
					case EConfirmScreenButtonType::Closed:
					inputActionRowHandle = ICommonInputModule::GetSettings().GetDefaultBackAction();
						break;
					default:
						break;
				}
				
				UFrontendCommonButtonBase* addedButton = DynamicEntryBox_Buttons->CreateEntry<UFrontendCommonButtonBase>();
				addedButton->SetButtonText(ButtonInfo.ButtonTextToDisplay);
				addedButton->SetTriggeredInputAction(inputActionRowHandle);
				addedButton->OnClicked().AddLambda(
					[InConfirmScreenCallback, ButtonInfo, this]()
					{
						InConfirmScreenCallback(ButtonInfo.ButtonType);
						DeactivateWidget();
					}	
				);
			}
			if (DynamicEntryBox_Buttons->GetNumEntries() != 0)
			{
				DynamicEntryBox_Buttons->GetAllEntries().Last()->SetFocus();
			}
		}
		
	}
	else
	{
		DebugHelper::Print(FString("InScreenInfoObject->InitConfirmScreen()"));
	}
}
