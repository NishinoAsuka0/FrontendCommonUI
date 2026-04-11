// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Components/FrontendTabListWidgetBase.h"
#include "Editor/WidgetCompilerLog.h"

void UFrontendTabListWidgetBase::RequestRegisterTab(const FName& InTabID, const FText& InDisplayName)
{
	RegisterTab(InTabID, TabButtonEntryWidgetClass, nullptr);
	
	if (UFrontendCommonButtonBase* frontendButton = Cast<UFrontendCommonButtonBase>(GetTabButtonBaseByID(InTabID)))
	{
		frontendButton->SetButtonText(InDisplayName);
	}
	
}

#if WITH_EDITOR
void UFrontendTabListWidgetBase::ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledDefaults(CompileLog);
	
	if (!TabButtonEntryWidgetClass)
	{
		CompileLog.Error(FText::FromString(
			TEXT("The variable TabButtonEntryWidgetClass has no valid entry specified. ") + 
			GetClass()->GetName() + 
			TEXT(" needs a valid entry widget class to function property")
			));
	}
}
#endif
