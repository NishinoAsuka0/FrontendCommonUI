// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonListView.h"
#include "Editor/WidgetCompilerLog.h"
#include "Widgets/Options/DataAsset_DataListEntryMapping.h"
#include "FrontendCommonListView.generated.h"

/**
 * 前端通用列表视图
 * 通过 DataListEntryMapping DataAsset 实现数据类型到控件类的自动映射
 */
UCLASS()
class FRONTENDUI_API UFrontendCommonListView : public UCommonListView
{
	GENERATED_BODY()
protected:
	/** 重写列表项创建逻辑：通过 DataAsset 映射查找数据对象对应的控件类 */
	virtual UUserWidget& OnGenerateEntryWidgetInternal(UObject* Item, TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable) override;
private:
#if WITH_EDITOR
	/** 编辑器编译时验证 DataListEntryMapping 是否已配置 */
	virtual void ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const override;
#endif
public:
	/** 数据对象类型 → 列表项控件类 的映射 DataAsset */
	UPROPERTY(EditAnywhere, Category = "Frontend List View Settings")
	UDataAsset_DataListEntryMapping* DataListEntryMapping;
};
