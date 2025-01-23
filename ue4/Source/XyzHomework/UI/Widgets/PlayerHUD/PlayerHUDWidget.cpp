// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUDWidget.h"

#include "ReticleWidget.h"
#include "WeaponAmmoWidget.h"
#include "CharacterAttributesWidget.h"
#include "Blueprint/WidgetTree.h"
#include "UI/Widgets/InteractableWidget.h"

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

void UPlayerHUDWidget::SetInteractableKeyText(FName KeyName)
{
	if (IsValid(InteractableWidget))
	{
		InteractableWidget->SetKeyName(KeyName);
	}
}

void UPlayerHUDWidget::ShowInteractableKey(bool bIsVisible)
{
	if (!IsValid(InteractableWidget))
	{
		return;
	}

	if (bIsVisible)
	{
		InteractableWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		InteractableWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

