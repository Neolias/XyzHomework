// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactive/Interactable.h"
#include "GameFramework/Actor.h"
#include "PickupItem.generated.h"

UCLASS(Abstract, NotBlueprintable)
class XYZHOMEWORK_API APickupItem : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	const FName& GetDataTableID() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Category = "Interactable | Pickup"))
	FName DataTableID = NAME_None;
};
