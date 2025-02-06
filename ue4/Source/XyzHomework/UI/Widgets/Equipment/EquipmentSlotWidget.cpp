// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentSlotWidget.h"

#include "Actors/Equipment/EquipmentItem.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Inventory/Items/InventoryItem.h"
#include "UI/Widgets/Inventory/InventorySlotWidget.h"

void UEquipmentSlotWidget::InitializeSlot(TWeakObjectPtr<AEquipmentItem> EquipmentItem, int32 SlotIndex)
{
	if (!EquipmentItem.IsValid())
	{
		return;
	}

	LinkedEquipmentItem = EquipmentItem;
	if (EquipmentItem.IsValid())
	{
		LinkedInventoryItem = EquipmentItem->GetLinkedInventoryItem();
	}
	SlotIndexInComponent = SlotIndex;
}

void UEquipmentSlotWidget::UpdateView()
{
	if (LinkedInventoryItem.IsValid())
	{
		ItemIcon->SetBrushFromTexture(LinkedInventoryItem->GetDescription().Icon);
		ItemName->SetText(LinkedInventoryItem->GetDescription().Name);
	}
	else
	{
		ItemIcon->SetBrushFromTexture(nullptr);
		ItemName->SetText(FText::FromName(NAME_None));
	}
}

FReply UEquipmentSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!LinkedEquipmentItem.IsValid())
	{
		return FReply::Handled();
	}

	FEventReply Reply = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton);
	return Reply.NativeReply;
}

void UEquipmentSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	checkf(DragAndDropWidgetClass.Get() != nullptr, TEXT("UEquipmentSlotWidget::NativeOnDragDetected drag and drop widget is not defined"));

	if (!LinkedInventoryItem.IsValid())
	{
		return;
	}

	UDragDropOperation* DragOperation = Cast<UDragDropOperation>(UWidgetBlueprintLibrary::CreateDragDropOperation(UDragDropOperation::StaticClass()));

	/* Some simplification for not define new widget for drag and drop operation  */
	UInventorySlotWidget* DragWidget = CreateWidget<UInventorySlotWidget>(GetOwningPlayer(), DragAndDropWidgetClass);
	DragWidget->SetItemIcon(LinkedInventoryItem->GetDescription().Icon);

	DragOperation->DefaultDragVisual = DragWidget;
	DragOperation->Pivot = EDragPivot::CenterCenter;
	DragOperation->Payload = LinkedInventoryItem.Get();
	OutOperation = DragOperation;

	LinkedEquipmentItem.Reset();
	LinkedInventoryItem.Reset();
	OnEquipmentRemoveFromSlot.ExecuteIfBound(SlotIndexInComponent);

	UpdateView();
}

bool UEquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	const UInventoryItem* InventoryItem = Cast<UInventoryItem>(InOperation->Payload);
	if (IsValid(InventoryItem))
	{
		return OnEquipmentDropInSlot.Execute(InventoryItem->GetEquipmentItemClass(), SlotIndexInComponent);
	}
	return false;
}

void UEquipmentSlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (!LinkedEquipmentItem.IsValid())
	{
		return;
	}

	LinkedInventoryItem = Cast<UInventoryItem>(InOperation->Payload);
	LinkedEquipmentItem->SetLinkedInventoryItem(LinkedInventoryItem);
	if (LinkedInventoryItem.IsValid())
	{
		OnEquipmentDropInSlot.Execute(LinkedInventoryItem->GetEquipmentItemClass(), SlotIndexInComponent);
	}
}