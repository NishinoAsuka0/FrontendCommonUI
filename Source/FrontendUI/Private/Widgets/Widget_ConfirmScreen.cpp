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

	// 创建 OK 按钮配置
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

	// 创建 Yes 按钮
	FConfirmScreenButtonInfo YesButtonInfo;
	YesButtonInfo.ButtonType = EConfirmScreenButtonType::Confirm;
	YesButtonInfo.ButtonTextToDisplay = FText::FromString(TEXT("Yes"));

	// 创建 No 按钮
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

	// 创建 OK 按钮
	FConfirmScreenButtonInfo OKButtonInfo;
	OKButtonInfo.ButtonType = EConfirmScreenButtonType::Confirm;
	OKButtonInfo.ButtonTextToDisplay = FText::FromString(TEXT("OK"));

	// 创建 Cancel 按钮
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
		// 设置标题和消息文本
		CommonTextBlock_Title->SetText(InScreenInfoObject->ScreenTitle);
		CommonTextBlock_Message->SetText(InScreenInfoObject->ScreenMessage);

		// 清除已有按钮绑定，防止重复注册
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
			// 根据配置动态创建按钮
			for (const FConfirmScreenButtonInfo& ButtonInfo : InScreenInfoObject->ConfirmScreenButtons)
			{
				// 为 Cancel 和 Closed 按钮绑定默认返回操作
				FDataTableRowHandle inputActionRowHandle;
				switch (ButtonInfo.ButtonType)
				{
					case EConfirmScreenButtonType::Cancelled:
						inputActionRowHandle = ICommonInputModule::GetSettings().GetDefaultBackAction();
					break;
					case EConfirmScreenButtonType::Closed:
						inputActionRowHandle = ICommonInputModule::GetSettings().GetDefaultBackAction();
						break;
					default:
						break;
				}

				// 创建按钮并设置文本、输入操作和点击回调
				UFrontendCommonButtonBase* addedButton = DynamicEntryBox_Buttons->CreateEntry<UFrontendCommonButtonBase>();
				addedButton->SetButtonText(ButtonInfo.ButtonTextToDisplay);
				addedButton->SetTriggeringInputAction(inputActionRowHandle);
				addedButton->OnClicked().AddLambda(
					[InConfirmScreenCallback, ButtonInfo, this]()
					{
						// 点击后触发外部回调并关闭弹窗
						InConfirmScreenCallback(ButtonInfo.ButtonType);
						DeactivateWidget();
					}
				);
			}
			// 聚焦最后一个按钮
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
