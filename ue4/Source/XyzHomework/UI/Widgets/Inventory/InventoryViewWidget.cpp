// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryViewWidget.h"

#include "InventorySlotWidget.h"
#include "Components/GridPanel.h"

void UInventoryViewWidget::InitializeWidget(TArray<FInventorySlot>& InventorySlots)
{
	for (FInventorySlot& Item : InventorySlots)
	{
		AddSlotToView(Item);
	}
}

void UInventoryViewWidget::AddSlotToView(FInventorySlot& SlotToAdd)
{
	checkf(InventorySlotWidgetClass.Get() != nullptr, TEXT("UItemContainerWidget::AddItemSlotView widget class doesn't not exist"));

	UInventorySlotWidget* SlotWidget = CreateWidget<UInventorySlotWidget>(this, InventorySlotWidgetClass);

	if (SlotWidget != nullptr)
	{
		SlotWidget->InitializeSlot(SlotToAdd);

		const int32 CurrentSlotCount = ItemSlots->GetChildrenCount();
		const int32 CurrentSlotRow = CurrentSlotCount / ColumnCount;
		const int32 CurrentSlotColumn = CurrentSlotCount % ColumnCount;
		ItemSlots->AddChildToGrid(SlotWidget, CurrentSlotRow, CurrentSlotColumn);

		SlotWidget->UpdateView();
	}
}