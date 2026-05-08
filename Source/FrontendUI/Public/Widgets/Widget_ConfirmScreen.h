// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Widget_ActivatableBase.h"
#include "CommonTextBlock.h"
#include "Components/DynamicEntryBox.h"
#include "FrontendTypes/FrontendEnumTypes.h"
#include "Widget_ConfirmScreen.generated.h"

/** 确认弹窗单个按钮的配置信息 */
USTRUCT(BlueprintType)
struct FConfirmScreenButtonInfo
{
	GENERATED_BODY()
	/** 按钮类型（Confirm/Cancelled/Closed） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EConfirmScreenButtonType ButtonType = EConfirmScreenButtonType::Unknown;

	/** 按钮显示文本 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ButtonTextToDisplay;
};

/**
 * 确认弹窗信息对象
 * 将弹窗配置（标题、消息、按钮）与弹窗控件解耦，支持工厂方法创建不同类型的弹窗
 */
UCLASS()
class FRONTENDUI_API UConfirmScreenInfoObject : public UObject
{
	GENERATED_BODY()
public:
	/** 创建只有 OK 按钮的弹窗配置 */
	static UConfirmScreenInfoObject* CreateOKScreen(const FText&InScreenTitle, const FText& InScreenMessage);
	/** 创建 Yes/No 双按钮弹窗配置 */
	static UConfirmScreenInfoObject* CreateYesNoScreen(const FText&InScreenTitle, const FText& InScreenMessage);
	/** 创建 OK/Cancel 双按钮弹窗配置 */
	static UConfirmScreenInfoObject* CreateOKCancelScreen(const FText&InScreenTitle, const FText& InScreenMessage);

	UPROPERTY(Transient)
	FText ScreenTitle;

	UPROPERTY(Transient)
	FText ScreenMessage;

	UPROPERTY(Transient)
	TArray<FConfirmScreenButtonInfo> ConfirmScreenButtons;
};

/**
 * 确认弹窗控件
 * 支持动态按钮创建和用户点击回调，通过 DynamicEntryBox 按配置自动生成按钮
 */
UCLASS(Abstract, BlueprintType, meta = (DisableNativeTick))
class FRONTENDUI_API UWidget_ConfirmScreen : public UWidget_ActivatableBase
{
	GENERATED_BODY()
public:
	/** 初始化弹窗：设置标题/消息，动态创建按钮并绑定点击回调 */
	void InitConfirmScreen(UConfirmScreenInfoObject*InScreenInfoObject, TFunction<void(EConfirmScreenButtonType)>InConfirmScreenCallback);
private:
	UPROPERTY(meta = (BindWidget))
	UCommonTextBlock* CommonTextBlock_Title;

	UPROPERTY(meta = (BindWidgetOptional))
	UCommonTextBlock* CommonTextBlock_Message;

	UPROPERTY(meta = (BindWidget))
	UDynamicEntryBox* DynamicEntryBox_Buttons;
};
