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

	void CreateViewWidget(APlayerController* PlayerController, UDataTable* InventoryItemDataTable);
	void OpenViewInventory(APlayerController* PlayerController, UDataTable* InventoryItemDataTable);
	void CloseViewInventory();
	bool IsViewInventoryVisible() const;
	bool AddInventoryItem(EInventoryItemType ItemType, int32 Amount, UDataTable* InventoryItemDataTable);
	void RemoveInventoryItem(EInventoryItemType ItemType, int32 Amount);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (ClampMin = 0, UIMin = 0))
	int32 Capacity = 16;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TSubclassOf<UInventoryViewWidget> InventoryViewWidgetClass;

private:
	int32 UsedSlotCount = 0;
	TArray<FInventorySlot> ItemSlots;
	UPROPERTY()
	UInventoryViewWidget* InventoryViewWidget;
	UPROPERTY()
	UDataTable* CachedItemDataTable;
};
