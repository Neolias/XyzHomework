// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactive/PickupItems/PickupItem.h"
#include "AmmoPickupItem.generated.h"

/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API AAmmoPickupItem : public APickupItem
{
	GENERATED_BODY()

public:
	virtual void Interact(APawn* InteractingPawn) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable | Ammo")
	EWeaponAmmoType AmmoType = EWeaponAmmoType::None;

};
