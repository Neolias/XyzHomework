// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Engine/DataTable.h"
#include "UObject/NoExportTypes.h"
#include "InventoryItem.generated.h"

class UEquipmentSlotWidget;
class UInventorySlotWidget;
class UInventoryItem;
class AEquipmentItem;
class APickupItem;

USTRUCT(BlueprintType)
struct FInventoryItemDescription : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EInventoryItemType InventoryItemType = EInventoryItemType::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<APickupItem> PickUpItemClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AEquipmentItem> EquipmentItemClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanStackItems = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bCanStackItems"))
	int32 MaxCount = 1;
};

USTRUCT(BlueprintType)
struct FInventoryTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UInventoryItem> InventoryItemClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FInventoryItemDescription InventoryItemDescription;
};

UCLASS(NotBlueprintable)
class XYZHOMEWORK_API UInventoryItem : public UObject
{
	GENERATED_BODY()

public:
	virtual void InitializeItem(const FInventoryItemDescription& InventoryItemDescription);
	virtual const FInventoryItemDescription& GetItemDescription() const { return Description; }
	virtual EInventoryItemType GetInventoryItemType() const { return Description.InventoryItemType; }
	virtual TSubclassOf<AEquipmentItem> GetEquipmentItemClass() const { return Description.EquipmentItemClass; }
	virtual bool CanStackItems() const { return Description.bCanStackItems; }
	virtual int32 GetMaxCount() const { return Description.MaxCount; }
	virtual bool IsEquipment() const { return bIsEquipment; }
	virtual int32 GetCount() const { return Count; }
	virtual void SetCount(const int32 NewCount);
	virtual int32 AddCount(int32 Value);
	virtual int32 GetAvailableSpaceInStack() const;
	virtual UInventorySlotWidget* GetPreviousInventorySlotWidget() const { return PreviousInventorySlotWidget; }
	virtual UEquipmentSlotWidget* GetPreviousEquipmentSlotWidget() const { return PreviousEquipmentSlotWidget; }
	virtual void SetPreviousInventorySlotWidget(UInventorySlotWidget* SlotWidget);
	virtual void SetPreviousEquipmentSlotWidget(UEquipmentSlotWidget* SlotWidget);
	virtual bool Consume(APawn* Pawn) { return false; }
	virtual bool AddToEquipment(APawn* Pawn);
	virtual bool RemoveFromEquipment(APawn* Pawn, int32 EquipmentSlotIndex);
	virtual void DropItem(APawn* Pawn);

protected:
	FInventoryItemDescription Description;
	bool bIsEquipment = false;
	int32 Count = 0;
	UPROPERTY()
	UInventorySlotWidget* PreviousInventorySlotWidget = nullptr;
	UPROPERTY()
	UEquipmentSlotWidget* PreviousEquipmentSlotWidget = nullptr;
};
