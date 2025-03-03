// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "XyzCharacterAttributeSet.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnStaminaChangedEventNew, float)

/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API UXyzCharacterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

	FOnStaminaChangedEventNew OnStaminaChangedEvent;

protected:	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes | Stamina")
	FGameplayAttributeData Stamina = 100.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes | Stamina")
	FGameplayAttributeData MaxStamina = 100.f;
};
