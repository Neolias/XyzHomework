// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Engine/DataTable.h"
#include "UObject/NoExportTypes.h"
#include "InventoryItem.generated.h"

class UInventoryItem;
class AEquipmentItem;
class APickupItem;

USTRUCT(BlueprintType)
struct FInventoryItemDescription : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* Icon;
};

USTRUCT(BlueprintType)
struct FInventoryTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AEquipmentItem> EquipmentItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UInventoryItem> InventoryItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FInventoryItemDescription InventoryItemDescription;
};

UCLASS(NotBlueprintable)
class XYZHOMEWORK_API UInventoryItem : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(EInventoryItemType ItemType_In, const FInventoryItemDescription& Description_In, TSubclassOf<AEquipmentItem> EquipmentItemClass_In);
	bool IsInitialized() const { return bIsInitialized; }
	EInventoryItemType GetItemType() const { return ItemType; }
	TSubclassOf<AEquipmentItem> GetEquipmentItemClass() const { return EquipmentItemClass; }
	const FInventoryItemDescription& GetDescription() const { return Description; }
	int32 GetCount() const { return Count; }
	void SetCount(const int32 NewCount) { Count = NewCount; }
	virtual bool Consume(APawn* Pawn) { return false; }
	virtual bool AddToEquipment(class AXyzBaseCharacter* BaseCharacter, EEquipmentItemSlot EquipmentItemSlot);

protected:
	EInventoryItemType ItemType = EInventoryItemType::None;
	TSubclassOf<AEquipmentItem> EquipmentItemClass;

private:
	FInventoryItemDescription Description;
	bool bIsInitialized = false;
	int32 Count = 0;
};
