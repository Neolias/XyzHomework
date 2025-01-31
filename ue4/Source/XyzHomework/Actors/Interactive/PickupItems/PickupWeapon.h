// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactive/PickupItems/PickupItem.h"
#include "PickupWeapon.generated.h"


UCLASS(Blueprintable)
class XYZHOMEWORK_API APickupWeapon : public APickupItem
{
	GENERATED_BODY()

public:
	APickupWeapon();
	virtual FName GetActionName() override;
	virtual void Interact(APawn* InteractingPawn) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Interactable"))
	FName ActionName = FName("InteractWithObject");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Interactable | Pickup"))
	UStaticMeshComponent* WeaponMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Interactable | Pickup"))
	TSubclassOf<class ARangedWeaponItem> RangedWeaponClass;
};
