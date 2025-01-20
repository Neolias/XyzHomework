// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/CharacterAttributesWidget.h"

void UCharacterAttributesWidget::OnHealthChanged(const float NewHealthPercentage)
{
	HealthPercentage = NewHealthPercentage;
}

void UCharacterAttributesWidget::OnStaminaChanged(const float NewStaminaPercentage)
{
	StaminaPercentage = NewStaminaPercentage;
	if (StaminaPercentage < 1.f)
	{
		bIsStaminaBarVisible = true;
	}
	else
	{
		bIsStaminaBarVisible = false;
	}
}

void UCharacterAttributesWidget::OnOxygenChanged(const float NewOxygenPercentage)
{
	OxygenPercentage = NewOxygenPercentage;
	if (OxygenPercentage < 1.f)
	{
		bIsOxygenBarVisible = true;
	}
	else
	{
		bIsOxygenBarVisible = false;
	}
}
