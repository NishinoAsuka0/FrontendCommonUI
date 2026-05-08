// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Options/DataObjects/ListDataObject_Collection.h"

void UListDataObject_Collection::AddChildListData(UListDataObject_Base* DataObject)
{
	// 通知子序列初始化自己（设置默认值等）
	DataObject->InitDataObject();

	// 设置子序列的 Parent 引用
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
