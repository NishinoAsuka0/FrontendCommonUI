// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * FrontendUI 模块
 * 负责注册编辑器 Tab（DataTable Importer）和管理模块生命周期
 */
class FFrontendUIModule : public IModuleInterface
{
public:
	/** 模块启动：注册编辑器 Tab */
	virtual void StartupModule() override;
	/** 模块关闭：取消注册编辑器 Tab */
	virtual void ShutdownModule() override;

private:
	/** 注册 DataTable Importer 可停靠 Tab 和 Window 菜单入口 */
	void RegisterEditorTab();
	/** 取消注册 DataTable Importer Tab */
	void UnregisterEditorTab();
	/** Tab 注册句柄，用于反注册 */
	FDelegateHandle TabSpawnerHandle;
};
