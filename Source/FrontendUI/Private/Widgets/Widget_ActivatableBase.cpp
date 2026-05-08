// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Widget_ActivatableBase.h"

// ECommonInputMode 枚举（Menu / Game / All）
#include "CommonInputModeTypes.h"
// EMouseCaptureMode / EMouseLockMode 引擎枚举
#include "Engine/EngineBaseTypes.h"

TOptional<FUIInputConfig> UWidget_ActivatableBase::GetDesiredInputConfig() const
{
	switch (WidgetInputMode)
	{
	case EFrontendWidgetInputMode::GameOnly:
		// 鼠标轴进 EnhancedInput；光标 capture + lock，避免移到副屏丢失视角控制
		return FUIInputConfig(
			ECommonInputMode::Game,
			EMouseCaptureMode::CapturePermanently,
			EMouseLockMode::LockAlways,
			/*bHideCursorDuringViewportCapture=*/true);

	case EFrontendWidgetInputMode::MenuOnly:
		// 鼠标完全交给 UI；不 capture、不 lock、显示光标
		return FUIInputConfig(
			ECommonInputMode::Menu,
			EMouseCaptureMode::NoCapture,
			EMouseLockMode::DoNotLock,
			/*bHideCursorDuringViewportCapture=*/false);

	case EFrontendWidgetInputMode::GameAndMenu:
		// 半透明菜单/暂停面板：鼠标既驱动 UI 又驱动视角；点下时短暂 capture 拿轴
		return FUIInputConfig(
			ECommonInputMode::All,
			EMouseCaptureMode::CaptureDuringMouseDown,
			EMouseLockMode::LockInFullscreen,
			/*bHideCursorDuringViewportCapture=*/false);

	case EFrontendWidgetInputMode::Default:
	default:
		// 不覆写，让栈下方的 widget / PC 默认 InputMode 接管
		return Super::GetDesiredInputConfig();
	}
}

AFrontendPlayerController* UWidget_ActivatableBase::GetOwningFrontendPlayerController()
{
	if (!CachedOwningFrontendPlayerController.IsValid())
	{
		CachedOwningFrontendPlayerController = GetOwningPlayer<AFrontendPlayerController>();
	}
	return CachedOwningFrontendPlayerController.IsValid()?CachedOwningFrontendPlayerController.Get():nullptr;
}
