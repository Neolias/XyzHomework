// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

/**
 * 
 */

class UCharacterAttributesWidget;

UCLASS()
class XYZHOMEWORK_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	class UReticleWidget* GetReticleWidget();
	class UWeaponAmmoWidget* GetWeaponAmmoWidget();
	UCharacterAttributesWidget* GetCharacterAttributesWidget();
	UCharacterAttributesWidget* GetCharacterAttributesCenterWidget();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	FName ReticleWidgetName = "WBP_Reticle";
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	FName WeaponAmmoWidgetName = "WBP_WeaponAmmo";
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	FName CharacterAttributesWidgetName = "WBP_CharacterAttributes";
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	FName CharacterAttributesCenterWidgetName = "WBP_CharacterAttributesCenter";
	
};
