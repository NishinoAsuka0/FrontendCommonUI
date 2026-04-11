// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Components/FrontendCommonListView.h"

UUserWidget& UFrontendCommonListView::OnGenerateEntryWidgetInternal(UObject* Item,
	TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable)
{
	if (IsDesignTime())
	{
		return Super::OnGenerateEntryWidgetInternal(Item, DesiredEntryClass, OwnerTable);
	}
	if (TSubclassOf<UWidget_ListEntry_Base> FoundWidgetClass = DataListEntryMapping->FindEntryWidgetClassByDataObject(CastChecked<UListDataObject_Base>(Item)))
	{
		return GenerateTypedEntry<UWidget_ListEntry_Base>(FoundWidgetClass, OwnerTable);	
	}
	return Super::OnGenerateEntryWidgetInternal(Item, DesiredEntryClass, OwnerTable);
}

#if WITH_EDITOR
void UFrontendCommonListView::ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledDefaults(CompileLog);
	
	if (!DataListEntryMapping)
	{
		CompileLog.Error(FText::FromString(TEXT("No DataListEntryMapping ") + GetClass()->GetName() + TEXT(" needs a valid data asset to function properly")));
	}
}
#endif
