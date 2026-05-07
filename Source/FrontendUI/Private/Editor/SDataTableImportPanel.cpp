// Copyright Epic Games, Inc. All Rights Reserved.

#include "Editor/SDataTableImportPanel.h"

#if WITH_EDITOR
#include "AbilitySystem/SkillConfigTypes.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/DataTable.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	struct FTableConfig
	{
		FString JsonPrefix;
		FString AssetPath;
		UScriptStruct* RowStruct;
	};

	TArray<FTableConfig> GetTableConfigs()
	{
		return {
			{ TEXT("DT_SkillConfig"),  TEXT("/Game/DataTables/DT_SkillConfig"),  FSkillConfigRow::StaticStruct()  },
			{ TEXT("DT_SkillEffect"),  TEXT("/Game/DataTables/DT_SkillEffect"),  FSkillEffectRow::StaticStruct() },
			{ TEXT("DT_BuffConfig"),   TEXT("/Game/DataTables/DT_BuffConfig"),   FBuffConfigRow::StaticStruct()   },
		};
	}

	FString GetJsonDirectory()
	{
		return FPaths::ProjectContentDir() / TEXT("Config/Json");
	}

	int32 ImportJsonToDataTable(UDataTable* DataTable, const FString& JsonContent, UScriptStruct* RowStruct)
	{
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonContent);
		TArray<TSharedPtr<FJsonValue>> JsonArray;
		if (!FJsonSerializer::Deserialize(Reader, JsonArray) || JsonArray.Num() == 0)
		{
			return -1;
		}

		int32 Imported = 0;
		for (const TSharedPtr<FJsonValue>& Value : JsonArray)
		{
			const TSharedPtr<FJsonObject>* ObjPtr = nullptr;
			if (!Value->TryGetObject(ObjPtr)) continue;

			TSharedPtr<FJsonObject> Obj = *ObjPtr;
			FString RowNameStr;
			if (!Obj->TryGetStringField(TEXT("Name"), RowNameStr) || RowNameStr.IsEmpty()) continue;

			Obj->Values.Remove(TEXT("Name"));

			uint8* RowMemory = (uint8*)FMemory::Malloc(RowStruct->GetStructureSize());
			RowStruct->InitializeStruct(RowMemory);

			if (FJsonObjectConverter::JsonObjectToUStruct(Obj.ToSharedRef(), RowStruct, RowMemory, 0, 0))
			{
				DataTable->AddRow(FName(*RowNameStr), reinterpret_cast<const FTableRowBase&>(*RowMemory));
				++Imported;
			}

			RowStruct->DestroyStruct(RowMemory);
			FMemory::Free(RowMemory);
		}
		return Imported;
	}

	UDataTable* GetOrCreateDataTable(const FString& AssetPath, UScriptStruct* RowStruct)
	{
		UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *AssetPath);
		if (!DataTable)
		{
			const FString AssetName = FPaths::GetBaseFilename(AssetPath);
			UPackage* Package = CreatePackage(*AssetPath);
			if (!Package) return nullptr;
			DataTable = NewObject<UDataTable>(Package, *AssetName, RF_Public | RF_Standalone);
			FAssetRegistryModule::AssetCreated(DataTable);
		}
		else
		{
			DataTable->EmptyTable();
		}
		DataTable->RowStruct = RowStruct;
		return DataTable;
	}

	bool SaveDataTableAsset(UDataTable* DataTable, const FString& AssetPath)
	{
		DataTable->MarkPackageDirty();
		const FString PackageFilename = FPackageName::LongPackageNameToFilename(
			AssetPath, FPackageName::GetAssetPackageExtension());
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		return UPackage::SavePackage(DataTable->GetOutermost(), DataTable, *PackageFilename, SaveArgs);
	}
}

void SDataTableImportPanel::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(8)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
			[
				SNew(SButton).Text(FText::FromString(TEXT("Import All")))
				.OnClicked_Lambda([this]() { OnImportAllClicked(); return FReply::Handled(); })
			]
		]
		+ SVerticalBox::Slot().FillHeight(1).Padding(8, 0, 8, 8)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SAssignNew(LogText, STextBlock)
				.Text(FText::FromString(TEXT("Ready.\n")))
				.AutoWrapText(true)
			]
		]
	];

	LogLines.Add(TEXT("Ready."));
}

void SDataTableImportPanel::AppendLog(const FString& Message)
{
	LogLines.Add(Message);
	LogText->SetText(FText::FromString(FString::Join(LogLines, TEXT("\n"))));
}

void SDataTableImportPanel::OnImportAllClicked()
{
	LogLines.Empty();
	AppendLog(TEXT("=== DataTable Import ==="));

	const FString JsonDir = GetJsonDirectory();
	if (!IFileManager::Get().DirectoryExists(*JsonDir))
	{
		AppendLog(FString::Printf(TEXT("[Error] JSON dir not found: %s"), *JsonDir));
		return;
	}

	const TArray<FTableConfig> Configs = GetTableConfigs();
	int32 TotalRows = 0;

	for (const FTableConfig& Config : Configs)
	{
		const FString JsonPath = JsonDir / Config.JsonPrefix + TEXT(".json");
		if (!FPaths::FileExists(JsonPath))
		{
			AppendLog(FString::Printf(TEXT("[Skip]  %s - JSON missing"), *Config.JsonPrefix));
			continue;
		}

		FString JsonContent;
		if (!FFileHelper::LoadFileToString(JsonContent, *JsonPath))
		{
			AppendLog(FString::Printf(TEXT("[Fail]  %s - Read error"), *Config.JsonPrefix));
			continue;
		}

		UDataTable* DataTable = GetOrCreateDataTable(Config.AssetPath, Config.RowStruct);
		if (!DataTable)
		{
			AppendLog(FString::Printf(TEXT("[Fail]  %s - Create error"), *Config.JsonPrefix));
			continue;
		}

		const int32 Count = ImportJsonToDataTable(DataTable, JsonContent, Config.RowStruct);
		if (Count < 0)
		{
			AppendLog(FString::Printf(TEXT("[Fail]  %s - JSON parse error"), *Config.JsonPrefix));
			continue;
		}

		if (!SaveDataTableAsset(DataTable, Config.AssetPath))
		{
			AppendLog(FString::Printf(TEXT("[Fail]  %s - Save error"), *Config.JsonPrefix));
			continue;
		}

		TotalRows += Count;
		AppendLog(FString::Printf(TEXT("[OK]    %-20s  %3d rows"), *Config.JsonPrefix, Count));
	}

	AppendLog(FString::Printf(TEXT("\nTotal: %d rows imported."), TotalRows));
}
#endif
