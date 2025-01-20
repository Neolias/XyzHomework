// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/PlayerHUDWidget.h"

#include "ReticleWidget.h"
#include "WeaponAmmoWidget.h"
#include "CharacterAttributesWidget.h"
#include "Blueprint/WidgetTree.h"

UReticleWidget* UPlayerHUDWidget::GetReticleWidget()
{
	return WidgetTree->FindWidget<UReticleWidget>(ReticleWidgetName);
}

UWeaponAmmoWidget* UPlayerHUDWidget::GetWeaponAmmoWidget()
{
	return WidgetTree->FindWidget<UWeaponAmmoWidget>(WeaponAmmoWidgetName);
}

UCharacterAttributesWidget* UPlayerHUDWidget::GetCharacterAttributesWidget()
{
	return WidgetTree->FindWidget<UCharacterAttributesWidget>(CharacterAttributesWidgetName);
}

UCharacterAttributesWidget* UPlayerHUDWidget::GetCharacterAttributesCenterWidget()
{
	return WidgetTree->FindWidget<UCharacterAttributesWidget>(CharacterAttributesCenterWidgetName);
}
