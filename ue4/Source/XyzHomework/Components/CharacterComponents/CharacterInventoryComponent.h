// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Components/ActorComponent.h"
#include "CharacterInventoryComponent.generated.h"

class UDataTable;
class UInventoryItem;
class UInventoryViewWidget;

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()

public:
	DECLARE_DELEGATE(FInventorySlotUpdate)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<UInventoryItem> Item;

	void BindOnInventorySlotUpdate(const FInventorySlotUpdate& Callback) const;
	void UnbindOnInventorySlotUpdate();
	void UpdateSlotState();
	void ClearSlot();

private:
	mutable FInventorySlotUpdate OnInventorySlotUpdate;

};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class XYZHOMEWORK_API UCharacterInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterInventoryComponent();

	void SetInventoryItemDataTable(const TSoftObjectPtr<UDataTable>& NewDataTable) { InventoryItemDataTable = NewDataTable; }
	void CreateViewWidget(APlayerController* PlayerController);
	void OpenViewInventory(APlayerController* PlayerController);
	void CloseViewInventory();
	bool IsViewInventoryVisible() const;
	bool AddInventoryItem(EInventoryItemType ItemType, int32 Amount);
	void RemoveInventoryItem(EInventoryItemType ItemType, int32 Amount);
	void RemoveInventoryItem(int32 SlotIndex, int32 Amount);
	void RemoveInventoryItem(FInventorySlot* Slot, int32 Amount);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (ClampMin = 0, UIMin = 0))
	int32 Capacity = 24;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TSubclassOf<UInventoryViewWidget> InventoryViewWidgetClass;

private:
	int32 StackItems(EInventoryItemType ItemType, int32 Amount);
	bool FillEmptySlots(EInventoryItemType ItemType, int32 Amount);

	int32 UsedSlotCount = 0;
	TArray<FInventorySlot> ItemSlots;
	UPROPERTY()
	UInventoryViewWidget* InventoryViewWidget;
	TSoftObjectPtr<UDataTable> InventoryItemDataTable;

};
