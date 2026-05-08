// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonTabListWidgetBase.h"
#include "FrontendCommonButtonBase.h"
#include "FrontendTabListWidgetBase.generated.h"

/**
 * 前端通用 Tab 列表控件
 * 封装 Tab 注册逻辑，注册时自动设置按钮文本
 */
UCLASS(Abstract, BlueprintType, meta = (DisableNativeTick))
class FRONTENDUI_API UFrontendTabListWidgetBase : public UCommonTabListWidgetBase
{
	GENERATED_BODY()
public:
	/** 注册 Tab 并设置按钮显示文本（封装 RegisterTab + SetButtonText 两步） */
	void RequestRegisterTab(const FName& InTabID, const FText& InDisplayName);
protected:
#if WITH_EDITOR
	/** 编辑器编译时验证 TabButtonEntryWidgetClass 是否已配置 */
	virtual void ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const override;
#endif
private:
	/** 编辑器预览用的调试 Tab 数量 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Frontend Tab List Settings", meta = (AllowPrivateAccess = true, ClampMin = "1", ClampMax = "10"))
	int32 DebugEditorPreviewTabCount = 3;

	/** Tab 按钮控件类 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Frontend Tab List Settings", meta = (AllowPrivateAccess = true))
	TSubclassOf<UFrontendCommonButtonBase>TabButtonEntryWidgetClass;
};
