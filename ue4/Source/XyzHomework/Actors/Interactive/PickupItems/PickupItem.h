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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interactable")
	EInventoryItemType ItemType = EInventoryItemType::None;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interactable", meta = (ClampMin = 1, UIMin = 1))
	int32 Amount = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
	UStaticMeshComponent* ItemMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
	FName ActionName = FName("InteractWithObject");

};
