// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AttributeSets/XyzCharacterAttributeSet.h"

void UXyzCharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	if (Attribute.AttributeName == FString("Stamina"))
	{
		NewValue = FMath::Clamp(NewValue, 0.f, MaxStamina.GetCurrentValue());
		if (OnStaminaChangedEvent.IsBound())
		{
			OnStaminaChangedEvent.Broadcast(NewValue / MaxStamina.GetBaseValue());
		}
	}
}

void UXyzCharacterAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	if (Attribute.AttributeName == FString("Stamina"))
	{
		NewValue = FMath::Clamp(NewValue, 0.f, MaxStamina.GetBaseValue());
		if (OnStaminaChangedEvent.IsBound())
		{
			OnStaminaChangedEvent.Broadcast(NewValue / MaxStamina.GetBaseValue());
		}
	}
}
