// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Options/DataObjects/ListDataObject_Base.h"
#include "ListDataObject_Collection.generated.h"

/**
 * 集合类型数据对象
 * 作为 Tab 分组容器，持有子数据列表，自动初始化子数据对象
 */
UCLASS()
class FRONTENDUI_API UListDataObject_Collection : public UListDataObject_Base
{
	GENERATED_BODY()
public:
	/** 添加子数据对象：触发 InitDataObject → 设置 Parent → 加入列表 */
	void AddChildListData(UListDataObject_Base* DataObject);

	// ---- UListDataObject_Base 接口 ----
	/** 获取所有子数据 */
	virtual TArray<UListDataObject_Base*> GetAllChildrenListData() const override;
	/** 是否有子数据 */
	virtual bool HasAnyChildrenListData() const override;
private:
	UPROPERTY(Transient)
	TArray<UListDataObject_Base*> ChildListDataArray;
};
