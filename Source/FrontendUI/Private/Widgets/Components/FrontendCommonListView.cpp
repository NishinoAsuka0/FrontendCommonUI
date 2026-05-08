// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Components/FrontendCommonListView.h"

UUserWidget& UFrontendCommonListView::OnGenerateEntryWidgetInternal(UObject* Item,
	TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable)
{
	// 设计时不使用自定义映射，走父类默认逻辑
	if (IsDesignTime())
	{
		return Super::OnGenerateEntryWidgetInternal(Item, DesiredEntryClass, OwnerTable);
	}
	// 通过 DataAsset 查找数据对象类型对应的控件类
	if (TSubclassOf<UWidget_ListEntry_Base> FoundWidgetClass = DataListEntryMapping->FindEntryWidgetClassByDataObject(CastChecked<UListDataObject_Base>(Item)))
	{
		return GenerateTypedEntry<UWidget_ListEntry_Base>(FoundWidgetClass, OwnerTable);
	}
	// 找不到映射时走父类默认逻辑
	return Super::OnGenerateEntryWidgetInternal(Item, DesiredEntryClass, OwnerTable);
}

#if WITH_EDITOR
void UFrontendCommonListView::ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledDefaults(CompileLog);

	// 编译时检查：DataListEntryMapping 必须配置
	if (!DataListEntryMapping)
	{
		CompileLog.Error(FText::FromString(TEXT("No DataListEntryMapping ") + GetClass()->GetName() + TEXT(" needs a valid data asset to function properly")));
	}
}
#endif
