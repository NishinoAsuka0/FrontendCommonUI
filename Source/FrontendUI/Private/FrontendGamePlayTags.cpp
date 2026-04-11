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
	
	UE_DEFINE_GAMEPLAY_TAG(Frontend_WidgetStack_OptionScreen, "Frontend.Widget.OptionScreen");
}
