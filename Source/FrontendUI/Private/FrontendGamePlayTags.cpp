// Fill out your copyright notice in the Description page of Project Settings.


#include "FrontendGamePlayTags.h"

namespace FrontendGameplayTags
{
	//Frontend widget stack
	UE_DEFINE_GAMEPLAY_TAG(Frontend_WidgetStack_Modal, "Frontend.WidgetStack.Modal");
	UE_DEFINE_GAMEPLAY_TAG(Frontend_WidgetStack_GameMenu, "Frontend.WidgetStack.GameMenu");
	UE_DEFINE_GAMEPLAY_TAG(Frontend_WidgetStack_HUD, "Frontend.WidgetStack.HUD");
	UE_DEFINE_GAMEPLAY_TAG(Frontend_WidgetStack_FrontEnd, "Frontend.WidgetStack.FrontEnd");
	
	//Frontend widgets
	UE_DEFINE_GAMEPLAY_TAG(Frontend_WidgetStack_PressAnyKeyScreen, "Frontend.Widget.PressAnyKeyScreen");
	UE_DEFINE_GAMEPLAY_TAG(Frontend_WidgetStack_MainMenuScreen, "Frontend.Widget.MainMenuScreen");
	UE_DEFINE_GAMEPLAY_TAG(Frontend_WidgetStack_ConfirmScreen, "Frontend.Widget.ConfirmScreen");
	UE_DEFINE_GAMEPLAY_TAG(Frontend_WidgetStack_StoryScreen, "Frontend.Widget.StoryScreen");
	UE_DEFINE_GAMEPLAY_TAG(Frontend_WidgetStack_GameHUDScreen, "Frontend.Widget.GameHUDScreen");
	
	UE_DEFINE_GAMEPLAY_TAG(Frontend_WidgetStack_OptionScreen, "Frontend.Widget.OptionScreen");
	UE_DEFINE_GAMEPLAY_TAG(Frontend_Image_TestImage, "Frontend.Image.TestImage");
	
	
	//HUD Widget

	// Skill system
	UE_DEFINE_GAMEPLAY_TAG(Skill_LightAttack, "Skill.LightAttack");
	UE_DEFINE_GAMEPLAY_TAG(Skill_HeavyAttack, "Skill.HeavyAttack");

	UE_DEFINE_GAMEPLAY_TAG(Effect_Damage, "Effect.Damage");
	UE_DEFINE_GAMEPLAY_TAG(Effect_Heal, "Effect.Heal");
	UE_DEFINE_GAMEPLAY_TAG(Effect_Buff, "Effect.Buff");

	UE_DEFINE_GAMEPLAY_TAG(Cost_HP, "Cost.HP");
	UE_DEFINE_GAMEPLAY_TAG(Cost_MP, "Cost.MP");
	UE_DEFINE_GAMEPLAY_TAG(Cost_Energy, "Cost.Energy");

	UE_DEFINE_GAMEPLAY_TAG(Target_Self, "Target.Self");
	UE_DEFINE_GAMEPLAY_TAG(Target_Enemy, "Target.Enemy");
	UE_DEFINE_GAMEPLAY_TAG(Target_Area, "Target.Area");

	UE_DEFINE_GAMEPLAY_TAG(Attr_HP, "Attr.HP");
	UE_DEFINE_GAMEPLAY_TAG(Attr_MP, "Attr.MP");
	UE_DEFINE_GAMEPLAY_TAG(Attr_Energy, "Attr.Energy");
}
