// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
// FUIInputConfig 是返回值类型（TOptional<T> 需要 T 的完整定义），必须 include 而非前向声明
#include "Input/UIActionBindingHandle.h"
#include "Controller/FrontendPlayerController.h"
#include "Widget_ActivatableBase.generated.h"

/**
 * 项目期望的输入模式（在 ClassDefaults 里配置）。
 * 包了一层是为了把 CommonUI 的 ECommonInputMode + EMouseCaptureMode + EMouseLockMode
 * 三段组合收敛成"对设计师有意义"的四种业务语义，避免每次都让人记三个枚举的搭配关系。
 *
 *  Default     - 不覆写，沿用 UCommonActivatableWidget 的默认（父类返回未指定时由栈下的 widget 决定）
 *  GameOnly    - 纯游戏模式：鼠标轴进 EnhancedInput，光标隐藏 + Capture（如 InGame HUD）
 *  MenuOnly    - 纯菜单模式：鼠标驱动 UI hover，角色不接收视角输入（如 PressAnyKey / MainMenu / Confirm）
 *  GameAndMenu - 混合模式：鼠标既能转视角又能点 UI（如暂停时半透明菜单、聊天框）
 */
UENUM(BlueprintType)
enum class EFrontendWidgetInputMode : uint8
{
	Default		UMETA(DisplayName = "Default (Inherit)"),
	GameOnly	UMETA(DisplayName = "Game Only"),
	MenuOnly	UMETA(DisplayName = "Menu Only"),
	GameAndMenu	UMETA(DisplayName = "Game And Menu")
};

/**
 *
 */
UCLASS(Abstract, BlueprintType, meta = (DisableNativeTick))
class FRONTENDUI_API UWidget_ActivatableBase : public UCommonActivatableWidget
{
	GENERATED_BODY()
public:
	// 覆写父类钩子：CommonUI 在 widget 激活时会调它拿 InputConfig 并应用到 PC，停用时自动 pop 还原。
	// 集中在基类做就不用每个子类各自切 InputMode，杜绝"开了 UI 后忘记切回来"的常见 bug。
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

protected:
	// 在 ClassDefaults 面板配置，决定本控件激活时把 PC 切到哪种 InputMode
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	EFrontendWidgetInputMode WidgetInputMode = EFrontendWidgetInputMode::Default;
	UFUNCTION(BlueprintPure)
	AFrontendPlayerController* GetOwningFrontendPlayerController();
private:
	TWeakObjectPtr<AFrontendPlayerController> CachedOwningFrontendPlayerController;


};
