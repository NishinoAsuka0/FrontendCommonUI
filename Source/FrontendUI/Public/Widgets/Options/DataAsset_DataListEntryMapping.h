// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataObjects/ListDataObject_Base.h"
#include "Engine/DataAsset.h"
#include "ListEntry/Widget_ListEntry_Base.h"
#include "DataAsset_DataListEntryMapping.generated.h"

/**
 * 数据列表项映射 DataAsset
 * 定义数据类型 → 列表项控件类的映射关系，可在编辑器中配置，无需改代码
 */
UCLASS()
class FRONTENDUI_API UDataAsset_DataListEntryMapping : public UDataAsset
{
	GENERATED_BODY()
public:
	/** 遍历数据对象的继承链，在映射表中查找匹配的控件类 */
	TSubclassOf<UWidget_ListEntry_Base> FindEntryWidgetClassByDataObject( UListDataObject_Base* ListItemObject);
private:
	/** 数据类型 → 控件类型 映射表 */
	UPROPERTY(EditDefaultsOnly)
	TMap<TSubclassOf<UListDataObject_Base>, TSubclassOf<UWidget_ListEntry_Base>> DataObjectListEntryMap;

};
