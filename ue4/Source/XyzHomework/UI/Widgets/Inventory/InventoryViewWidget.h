// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CharacterComponents/CharacterInventoryComponent.h"
#include "InventoryViewWidget.generated.h"

struct FInventorySlot;
class UInventorySlotWidget;
class UGridPanel;

UCLASS()
class XYZHOMEWORK_API UInventoryViewWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void InitializeWidget(TArray<FInventorySlot>& InventorySlots);

protected:
	UPROPERTY(meta = (BindWidget))
	UGridPanel* GridPanelItemSlots;

	UPROPERTY(EditDefaultsOnly, Category = "ItemSlots")
	TSubclassOf<UInventorySlotWidget> InventorySlotWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "ItemSlots")
	int32 ColumnCount = 4;

	void AddSlotToView(FInventorySlot& SlotToAdd);
};
