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
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* Icon;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCanStackItems = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bCanStackItems"))
	int32 MaxCount = 1;
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
	virtual void Initialize(EInventoryItemType ItemType_In, const FInventoryItemDescription& Description_In, TSubclassOf<AEquipmentItem> EquipmentItemClass_In = nullptr);
	virtual bool IsEquipment() const { return bIsEquipment; }
	virtual bool CanStackItems() const { return Description.bCanStackItems; }
	virtual EInventoryItemType GetItemType() const { return ItemType; }
	virtual TSubclassOf<AEquipmentItem> GetEquipmentItemClass() const { return EquipmentItemClass; }
	virtual const FInventoryItemDescription& GetDescription() const { return Description; }
	virtual int32 GetCount() const { return Count; }
	virtual void SetCount(const int32 NewCount);
	virtual int32 GetMaxCount() const { return Description.MaxCount; }
	virtual int32 AddCount(int32 Value);
	virtual int32 GetAvailableSpaceInStack() const;
	virtual UInventorySlotWidget* GetPreviousInventorySlotWidget() const { return PreviousInventorySlotWidget; }
	virtual UEquipmentSlotWidget* GetPreviousEquipmentSlotWidget() const { return PreviousEquipmentSlotWidget; }
	virtual void SetPreviousInventorySlotWidget(UInventorySlotWidget* SlotWidget);
	virtual void SetPreviousEquipmentSlotWidget(UEquipmentSlotWidget* SlotWidget);
	virtual bool Consume(APawn* Pawn) { return false; }
	virtual bool AddToEquipment(APawn* Pawn);
	virtual bool RemoveFromEquipment(APawn* Pawn, int32 EquipmentSlotIndex);

protected:
	EInventoryItemType ItemType = EInventoryItemType::None;
	FInventoryItemDescription Description;
	TSubclassOf<AEquipmentItem> EquipmentItemClass;
	bool bIsEquipment = false;
	int32 Count = 0;
	UPROPERTY()
	UInventorySlotWidget* PreviousInventorySlotWidget = nullptr;
	UPROPERTY()
	UEquipmentSlotWidget* PreviousEquipmentSlotWidget = nullptr;
};
