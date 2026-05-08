// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if WITH_EDITOR
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SBoxPanel.h"

class STextBlock;
class SScrollBox;

/**
 * DataTable 批量导入面板
 *
 * 从 Content/Config/Json/ 导入 JSON 到 Content/DataTables/
 */

struct FTableConfig
{
	FString JsonPrefix;
	FString AssetPath;
	UScriptStruct* RowStruct;
};

class SDataTableImportPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDataTableImportPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	void OnImportAllClicked();
	void OnChooseOneClicked();
	int LoadDataTable(FString JsonPath, const FTableConfig& Config);
	void AppendLog(const FString& Message);

	TSharedPtr<STextBlock> LogText;
	TArray<FString> LogLines;
};
#endif
