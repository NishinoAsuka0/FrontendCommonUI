// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Options/DataObjects/ListDataObject_Collection.h"

void UListDataObject_Collection::AddChildListData(UListDataObject_Base* DataObject)
{
	//通知子序列初始化自己
	DataObject->InitDataObject();
	
	//设置子序列的parent
	DataObject->SetParentData(this);
	ChildListDataArray.Add(DataObject);
}

TArray<UListDataObject_Base*> UListDataObject_Collection::GetAllChildrenListData() const
{
	return ChildListDataArray;
}

bool UListDataObject_Collection::HasAnyChildrenListData() const
{
	return !ChildListDataArray.IsEmpty();
}
