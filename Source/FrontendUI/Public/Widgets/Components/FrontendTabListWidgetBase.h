// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonTabListWidgetBase.h"
#include "FrontendCommonButtonBase.h"
#include "FrontendTabListWidgetBase.generated.h"

/**
 * 
 */
UCLASS(Abstract, BlueprintType, meta = (DisableNativeTick))
class FRONTENDUI_API UFrontendTabListWidgetBase : public UCommonTabListWidgetBase
{
	GENERATED_BODY()
public:
	void RequestRegisterTab(const FName& InTabID, const FText& InDisplayName);
protected:
#if WITH_EDITOR
	virtual void ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const override;
#endif
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Frontend Tab List Settings", meta = (AllowPrivateAccess = true, ClampMin = "1", ClampMax = "10"))
	int32 DebugEditorPreviewTabCount = 3;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Frontend Tab List Settings", meta = (AllowPrivateAccess = true))
	TSubclassOf<UFrontendCommonButtonBase>TabButtonEntryWidgetClass;
};
