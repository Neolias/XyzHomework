// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterAttributesWidget.generated.h"

/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API UCharacterAttributesWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float HealthPercentage = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float StaminaPercentage = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float OxygenPercentage = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	bool bIsStaminaBarVisible = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	bool bIsOxygenBarVisible = false;

	UFUNCTION()
	void OnHealthChanged(float NewHealthPercentage);
	UFUNCTION()
	void OnStaminaChanged(float NewStaminaPercentage);
	UFUNCTION()
	void OnOxygenChanged(float NewOxygenPercentage);
};
