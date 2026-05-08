// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FrontendTypes/FrontendEnumTypes.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "UObject/Object.h"
#include "ListDataObject_Base.generated.h"

// 为私有成员自动生成标准 Get/Set 方法，减少样板代码
#define LIST_DATA_ACCESSOR(DataType, PropertyName) \
	FORCEINLINE DataType Get##PropertyName() const { return PropertyName; }\
	void Set##PropertyName(DataType In##PropertyName){PropertyName = In##PropertyName;}

/**
 * 列表数据对象抽象基类
 * 定义 ID/名称/委托等通用成员，提供修改通知和 Apply 机制
 */
UCLASS(Abstract)
class FRONTENDUI_API UListDataObject_Base : public UObject
{
	GENERATED_BODY()
public:
	/** 数据修改委托：数据变更时通知 UI 层更新 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnListDataModifiedDelegate, UListDataObject_Base*, EOptionsListDataModifyReason);
	FOnListDataModifiedDelegate OnListDataModifiedDelegate;

	LIST_DATA_ACCESSOR(FName, DataID);
	LIST_DATA_ACCESSOR(FText, DataDisplayName);
	LIST_DATA_ACCESSOR(FText, DescriptionText);
	LIST_DATA_ACCESSOR(FText, DisableRichText);
	LIST_DATA_ACCESSOR(TSoftObjectPtr<UTexture2D>, SoftDescriptionImage);
	LIST_DATA_ACCESSOR(UListDataObject_Base*, ParentData);

	/** 设置是否每次修改都立即 ApplySettings（如音频音量需要即时生效） */
	void SetShouldAppySettingsImmediately(bool ShouldApplyRightNow) { bShouldApplyChangeImmediately = ShouldApplyRightNow; }

	/** 触发 OnDataObjectInitialized()，由 Collection 在 AddChild 时调用 */
	void InitDataObject();

	/** 子类重写以返回子数据列表 */
	virtual TArray<UListDataObject_Base*> GetAllChildrenListData() const { return TArray<UListDataObject_Base*>(); }
	/** 是否有子数据 */
	virtual bool HasAnyChildrenListData() const { return false; }

	/** 是否有默认值 */
	virtual bool HasDefaultValues() const { return false; }
	/** 当前值是否与默认值不同 */
	virtual bool CanResetBackToDefault() const { return false; }
	/** 尝试重置为默认值 */
	virtual bool TryResetBackToDefault(){ return false; }
protected:
	/** 子类重写以设置默认值 */
	virtual void OnDataObjectInitialized();

	/** 广播数据修改委托，若 bShouldApplyChangeImmediately 则自动 ApplySettings */
	virtual void NotifyListDataModified(UListDataObject_Base* ModifiedData, EOptionsListDataModifyReason ModifiedReason = EOptionsListDataModifyReason::DirectlyModified);
private:
	FName DataID;
	FText DataDisplayName;
	FText DescriptionText;
	FText DisableRichText;
	TSoftObjectPtr<UTexture2D> SoftDescriptionImage;

	UPROPERTY(Transient)
	UListDataObject_Base* ParentData;

	bool bShouldApplyChangeImmediately = false;
};
