// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Options/DataAsset_DataListEntryMapping.h"

TSubclassOf<UWidget_ListEntry_Base> UDataAsset_DataListEntryMapping::FindEntryWidgetClassByDataObject(
	UListDataObject_Base* ListItemObject)
{
	if (ListItemObject)
	{
		for (UClass* DataObjectClass = ListItemObject->GetClass(); DataObjectClass; DataObjectClass = DataObjectClass->GetSuperClass())
		{
			if (TSubclassOf<UListDataObject_Base> ConvertedDataObjectClass = TSubclassOf<UListDataObject_Base>(DataObjectClass))
			{
				if (DataObjectListEntryMap.Contains(ConvertedDataObjectClass))
				{
					return DataObjectListEntryMap.FindRef(ConvertedDataObjectClass);
				}
			}
		}
	}
	return TSubclassOf<UWidget_ListEntry_Base>();
}
