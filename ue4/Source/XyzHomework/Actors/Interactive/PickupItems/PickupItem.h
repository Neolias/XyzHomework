// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Actors/Interactive/Interactable.h"
#include "GameFramework/Actor.h"
#include "PickupItem.generated.h"

UCLASS()
class XYZHOMEWORK_API APickupItem : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	APickupItem();
	virtual FName GetActionName() override;
	EInventoryItemType GetItemType() const { return ItemType; }
	virtual void Interact(APawn* InteractingPawn) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Interactable | "))
	FName ActionName = FName("InteractWithObject");
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Category = "Interactable | Pickup"))
	EInventoryItemType ItemType = EInventoryItemType::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Interactable | Pickup"))
	UStaticMeshComponent* ItemMesh;

};
