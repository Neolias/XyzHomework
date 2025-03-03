// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAttributesWidget.h"

void UCharacterAttributesWidget::OnHealthChanged(const float NewHealthPercentage)
{
	HealthPercentage = NewHealthPercentage;
}

void UCharacterAttributesWidget::OnStaminaChanged(const float NewStaminaPercentage)
{
	StaminaPercentage = NewStaminaPercentage;
	bIsStaminaBarVisible = StaminaPercentage < 1.f ? true : false;
}

void UCharacterAttributesWidget::OnOxygenChanged(const float NewOxygenPercentage)
{
	OxygenPercentage = NewOxygenPercentage;
	bIsOxygenBarVisible = OxygenPercentage < 1.f ? true : false;
}
