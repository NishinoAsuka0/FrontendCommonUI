// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataObjects/ListDataObject_Collection.h"
#include "UObject/Object.h"
#include "OptionsDataRegistry.generated.h"

/**
 * 选项数据注册表
 * 集中注册所有 Tab 分组的选项数据，在 OptionScreen 首次激活时初始化
 */
UCLASS()
class FRONTENDUI_API UOptionsDataRegistry : public UObject
{
	GENERATED_BODY()
public:
	/** OptionScreen 创建后立即调用，初始化所有 Tab 分组数据 */
	void InitOptionsDataRegistry(ULocalPlayer* InOwningLocalPlayer);

	/** 获取所有已注册的 Tab 集合 */
	const TArray<UListDataObject_Collection*>& GetRegisteredOptionsTabCollections() const { return RegisteredOptionsTabCollections; };

	/** 按 TabID 查找对应的子数据列表 */
	TArray<UListDataObject_Base*>GetListSourceItemByTabID(const FName& InSelectedID);
private:
	/** 初始化 Gameplay Tab（含 Difficulty 等选项） */
	void InitGameplayCollectionTab();
	/** 初始化 Audio Tab */
	void InitAudioCollectionTab();
	/** 初始化 Video Tab */
	void InitVideoCollectionTab();
	/** 初始化 Control Tab */
	void InitControlCollectionTab();

	UPROPERTY(Transient)
	TArray<UListDataObject_Collection*> RegisteredOptionsTabCollections;
};
