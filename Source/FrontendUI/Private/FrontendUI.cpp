// Copyright Epic Games, Inc. All Rights Reserved.

#include "FrontendUI.h"
#include "Editor/SDataTableImportPanel.h"

#if WITH_EDITOR
#include "Framework/Docking/TabManager.h"
#include "ToolMenus.h"

namespace
{
	static const FName TabId("DataTableImporter");
}

void FFrontendUIModule::RegisterEditorTab()
{
	// 注册可停靠 Tab
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TabId,
		FOnSpawnTab::CreateLambda([](const FSpawnTabArgs&) -> TSharedRef<SDockTab>
		{
			return SNew(SDockTab)
				.TabRole(ETabRole::NomadTab)
				.Label(FText::FromString(TEXT("DataTable Importer")))
				[
					SNew(SDataTableImportPanel)
				];
		}))
		.SetDisplayName(FText::FromString(TEXT("DataTable Importer")))
		.SetTooltipText(FText::FromString(TEXT("Import JSON files from Content/Config/Json into DataTables.")))
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"));

	// 注册 Window 菜单入口
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]()
	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
		Section.AddMenuEntry(
			"DataTableImporter",
			FText::FromString(TEXT("DataTable Importer")),
			FText::FromString(TEXT("Open the DataTable Importer panel")),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([]()
			{
				FGlobalTabmanager::Get()->TryInvokeTab(TabId);
			}))
		);
	}));
}

void FFrontendUIModule::UnregisterEditorTab()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabId);
}

void FFrontendUIModule::StartupModule()
{
	RegisterEditorTab();
}

void FFrontendUIModule::ShutdownModule()
{
	UnregisterEditorTab();
}
#endif

IMPLEMENT_MODULE(FFrontendUIModule, FrontendUI)
