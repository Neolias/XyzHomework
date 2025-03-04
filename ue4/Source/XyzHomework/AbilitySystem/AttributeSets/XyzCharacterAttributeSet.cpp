// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AttributeSets/XyzCharacterAttributeSet.h"

void UXyzCharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	if (Attribute.AttributeName == FString("Stamina"))
	{
		NewValue = FMath::Clamp(NewValue, 0.f, MaxStamina.GetCurrentValue());
		OnStaminaChanged(NewValue);
	}
	if (Attribute.AttributeName == FString("Health"))
	{
		NewValue = FMath::Clamp(NewValue, 0.f, MaxHealth.GetCurrentValue());
		OnHealthChanged(NewValue);
	}
}

void UXyzCharacterAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	if (Attribute.AttributeName == FString("Stamina"))
	{
		NewValue = FMath::Clamp(NewValue, 0.f, MaxStamina.GetBaseValue());
		OnStaminaChanged(NewValue);
	}
	if (Attribute.AttributeName == FString("Health"))
	{
		NewValue = FMath::Clamp(NewValue, 0.f, MaxHealth.GetBaseValue());
		OnHealthChanged(NewValue);
	}
}
void UXyzCharacterAttributeSet::OnStaminaChanged(float NewValue) const
{
	if (OnStaminaChangedEvent.IsBound())
	{
		OnStaminaChangedEvent.Broadcast(NewValue / MaxStamina.GetBaseValue());
	}
}

void UXyzCharacterAttributeSet::OnHealthChanged(float NewValue) const
{
	if (OnHealthChangedEvent.IsBound())
	{
		OnHealthChangedEvent.Broadcast(NewValue / MaxStamina.GetBaseValue());
	}
}
