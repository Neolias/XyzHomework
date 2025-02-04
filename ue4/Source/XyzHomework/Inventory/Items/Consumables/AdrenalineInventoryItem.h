// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Items/InventoryItem.h"
#include "AdrenalineInventoryItem.generated.h"


UCLASS()
class XYZHOMEWORK_API UAdrenalineInventoryItem : public UInventoryItem
{
	GENERATED_BODY()

public:
	virtual bool Consume(APawn* Pawn) override;
};
