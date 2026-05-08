// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Widgets/Widget_ActivatableBase.h"
#include "GameplayTagContainer.h"
#include "FrontendTypes/FrontendEnumTypes.h"
#include "Widgets/Components/FrontendCommonButtonBase.h"
#include "FrontendUISubSystem.generated.h"

/**
 *
 */
class UWidget_PrimaryLayout;

/** 异步推送控件时的回调阶段 */
enum class EAsyncPushWidgetState : uint8
{
	OnCreatedBefore,  // 控件已创建但尚未推入 Stack，可在此阶段初始化控件数据
	AfterPush         // 控件已推入 Stack
};

/** 按钮描述文本更新委托：悬停按钮时广播，供底部提示栏等全局 UI 使用 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnButtonTextUpdatedDelegate, UFrontendCommonButtonBase*, BroadcastingButton, FText, DescriptionText);

/**
 * 前端 UI 子系统（GameInstance 级单例）
 * 集中管理所有 UI 控件的异步推送，生命周期跨越关卡切换
 */
UCLASS()
class FRONTENDUI_API UFrontendUISubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	/** 通过 WorldContextObject 获取 Subsystem 单例 */
	static UFrontendUISubSystem* Get(const UObject* UWorldContextObject);

	/** 仅非 DedicatedServer 实例创建 UI 子系统 */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	/** 蓝图调用：注册 PrimaryLayout 到缓存 */
	UFUNCTION(BlueprintCallable)
	void RegisterCreatedPrimaryLayoutWidget(UWidget_PrimaryLayout* InCreatedWidget);

	/** 异步加载控件类并按 Tag 推入对应 WidgetStack，通过 Callback 返回两个阶段的回调 */
	void PushSoftWidgetToStackAsync(const FGameplayTag& tag, TSoftClassPtr<UWidget_ActivatableBase> InSoftWidgetClass, TFunction<void(EAsyncPushWidgetState, UWidget_ActivatableBase*)> Callback);

	/** 创建确认弹窗信息对象并推入 Modal Stack */
	void PushConfirmScreenToModalStackAsync(EConfirmScreenType InScrennType, const FText& InScreenTitle, const FText& InScreenMsg, TFunction<void(EConfirmScreenButtonType)> Callback);

	/** 按钮悬停时广播描述文本，供底部提示栏订阅 */
	UPROPERTY(BlueprintAssignable)
	FOnButtonTextUpdatedDelegate OnButtonTextUpdatedDelegate;

private:
	UPROPERTY(Transient)
	UWidget_PrimaryLayout* CreatedPrimaryLayoutWidget = nullptr;
};
