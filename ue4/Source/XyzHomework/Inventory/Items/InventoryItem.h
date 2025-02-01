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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemView")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemView")
	UTexture2D* Icon;
};

USTRUCT(BlueprintType)
struct FWeaponTableRow : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponView")
	TSubclassOf<APickupItem> PickupActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponView")
	TSubclassOf<AEquipmentItem> EquipmentActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponView")
	FInventoryItemDescription WeaponItemDescription;
};

USTRUCT(BlueprintType)
struct FItemTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemView")
	TSubclassOf<APickupItem> PickupActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemView")
	TSubclassOf<UInventoryItem> InventoryItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemView")
	FInventoryItemDescription InventoryItemDescription;
};

UCLASS(Blueprintable)
class XYZHOMEWORK_API UInventoryItem : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(EInventoryItemType ItemType_In, const FInventoryItemDescription& Description_In);

	EInventoryItemType GetItemType() const { return ItemType; }
	const FInventoryItemDescription& GetDescription() const { return Description; }
	int32 GetCount() const { return Count; }
	void SetCount(const int32 NewCount) { Count = NewCount; }
	virtual void Consume(APawn* Pawn) {};

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "InventoryItem")
	EInventoryItemType ItemType = EInventoryItemType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "InventoryItem")
	FInventoryItemDescription Description;


private:
	bool bIsInitialized = false;
	int32 Count = 0;
};
