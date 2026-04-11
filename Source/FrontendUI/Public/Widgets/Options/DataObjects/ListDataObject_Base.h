// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "UObject/Object.h"
#include "ListDataObject_Base.generated.h"

#define LIST_DATA_ACCESSOR(DataType, PropertyName) \
	FORCEINLINE DataType Get##PropertyName() const { return PropertyName; }\
	void Set##PropertyName(DataType In##PropertyName){PropertyName = In##PropertyName;}
/**
 * 
 */
UCLASS(Abstract)
class FRONTENDUI_API UListDataObject_Base : public UObject
{
	GENERATED_BODY()
public:
	LIST_DATA_ACCESSOR(FName, DataID);
	LIST_DATA_ACCESSOR(FText, DataDisplayName);
	LIST_DATA_ACCESSOR(FText, DescriptionText);
	LIST_DATA_ACCESSOR(FText, DisableRichText);
	LIST_DATA_ACCESSOR(UListDataObject_Base*, ParentData);
	
	void InitDataObject();
	
	//Empty in base class ,child class ListDataObject_Collection should override it
	virtual  TArray<UListDataObject_Base*> GetAllChildrenListData() const { return TArray<UListDataObject_Base*>(); }
	virtual  bool HasAnyChildrenListData() const { return false; }
protected:
	virtual void OnDataObjectInitialized();
private:
	FName DataID;
	FText DataDisplayName;
	FText DescriptionText;
	FText DisableRichText;
	TSoftObjectPtr<UTexture2D> SoftDescriptionImage;
	
	UPROPERTY(Transient)
	UListDataObject_Base* ParentData;
};
