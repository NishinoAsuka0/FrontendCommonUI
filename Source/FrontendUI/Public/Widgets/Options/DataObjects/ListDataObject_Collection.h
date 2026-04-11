// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Options/DataObjects/ListDataObject_Base.h"
#include "ListDataObject_Collection.generated.h"

/**
 * 
 */
UCLASS()
class FRONTENDUI_API UListDataObject_Collection : public UListDataObject_Base
{
	GENERATED_BODY()
public:
	void AddChildListData(UListDataObject_Base* DataObject);
	
	//Begin UListDataObject_Base
	virtual  TArray<UListDataObject_Base*> GetAllChildrenListData() const override;
	virtual  bool HasAnyChildrenListData() const override;
	//end UListDataObject_Base
private:
	UPROPERTY(Transient)
	TArray<UListDataObject_Base*> ChildListDataArray;
};
