// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/DataTable.h"
#include "GameplayEffect.h"
#include "Widgets/Widget_ActivatableBase.h"
#include "FrontendDeveloperSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Frontend UI Settings"))
class FRONTENDUI_API UFrontendDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(Config, EditAnywhere, Category= "Widget Reference", meta = (ForceInlineRow, Categories = "Frontend.Widget"))
	TMap<FGameplayTag, TSoftClassPtr<UWidget_ActivatableBase>> FrontendWidgetMap;

	UPROPERTY(Config, EditAnywhere, Category= "Widget Reference", meta = (ForceInlineRow, Categories = "Frontend.Image"))
	TMap<FGameplayTag, TSoftObjectPtr<UTexture2D>> OptionsScreenSoftImageMap;

	// ---- 技能系统配置 ----

	UPROPERTY(Config, EditAnywhere, Category = "SkillSystem")
	TSoftObjectPtr<UDataTable> SkillConfigTable;

	UPROPERTY(Config, EditAnywhere, Category = "SkillSystem")
	TSoftObjectPtr<UDataTable> SkillEffectTable;

	UPROPERTY(Config, EditAnywhere, Category = "SkillSystem")
	TSoftObjectPtr<UDataTable> BuffConfigTable;

	UPROPERTY(Config, EditAnywhere, Category = "SkillSystem")
	TSubclassOf<UGameplayEffect> DamageGEClass;

	UPROPERTY(Config, EditAnywhere, Category = "SkillSystem")
	TSubclassOf<UGameplayEffect> HealGEClass;

	UPROPERTY(Config, EditAnywhere, Category = "SkillSystem")
	TSubclassOf<UGameplayEffect> BuffGEClass;
};
