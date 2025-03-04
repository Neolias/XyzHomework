// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "XyzCharacterAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API UXyzCharacterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnStaminaChangedEvent, float)
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnHealthChangedEvent, float)
	
public:
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

	FOnStaminaChangedEvent OnStaminaChangedEvent;
	FOnHealthChangedEvent OnHealthChangedEvent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes | Stamina")
	FGameplayAttributeData Stamina = 100.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes | Stamina")
	FGameplayAttributeData MaxStamina = 100.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes | Health")
	FGameplayAttributeData Health = 100.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes | Health")
	FGameplayAttributeData MaxHealth = 100.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes | Health")
	FGameplayAttributeData Defence = 2.f;

protected:
	void OnStaminaChanged(float NewValue) const;
	void OnHealthChanged(float NewValue) const;
};
