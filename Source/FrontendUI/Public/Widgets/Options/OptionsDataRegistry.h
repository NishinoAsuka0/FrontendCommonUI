// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataObjects/ListDataObject_Collection.h"
#include "UObject/Object.h"
#include "OptionsDataRegistry.generated.h"

/**
 * 
 */
UCLASS()
class FRONTENDUI_API UOptionsDataRegistry : public UObject
{
	GENERATED_BODY()
public:
	//Gets Called by options screen right after the object of type UOptionDataRegistry is Created
	void InitOptionsDataRegistry(ULocalPlayer* InOwningLocalPlayer);
	
	const TArray<UListDataObject_Collection*>& GetRegisteredOptionsTabCollections() const { return RegisteredOptionsTabCollections; };
	
	TArray<UListDataObject_Base*>GetListSourceItemByTabID(const FName& InSelectedID);
private:
	void InitGameplayCollectionTab();
	void InitAudioCollectionTab();	
	void InitVideoCollectionTab();
	void InitControlCollectionTab();
	
	UPROPERTY(Transient)
	TArray<UListDataObject_Collection*> RegisteredOptionsTabCollections;
};
