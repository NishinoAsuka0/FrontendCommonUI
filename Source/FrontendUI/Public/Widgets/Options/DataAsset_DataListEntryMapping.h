// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataObjects/ListDataObject_Base.h"
#include "Engine/DataAsset.h"
#include "ListEntry/Widget_ListEntry_Base.h"
#include "DataAsset_DataListEntryMapping.generated.h"

/**
 * 
 */
UCLASS()
class FRONTENDUI_API UDataAsset_DataListEntryMapping : public UDataAsset
{
	GENERATED_BODY()
public:
	TSubclassOf<UWidget_ListEntry_Base> FindEntryWidgetClassByDataObject( UListDataObject_Base* ListItemObject);
private:
	UPROPERTY(EditDefaultsOnly)
	TMap<TSubclassOf<UListDataObject_Base>, TSubclassOf<UWidget_ListEntry_Base>> DataObjectListEntryMap;
	
};
