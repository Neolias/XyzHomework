// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentSlotWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Inventory/Items/InventoryItem.h"
#include "UI/Widgets/Inventory/InventorySlotWidget.h"

void UEquipmentSlotWidget::InitializeSlot(TWeakObjectPtr<UInventoryItem> InventoryItem, int32 SlotIndex)
{
	LinkedInventoryItem = InventoryItem;
	SlotIndexInComponent = SlotIndex;
	SlotName->SetText(UEnum::GetDisplayValueAsText<EEquipmentItemSlot>((EEquipmentItemSlot)SlotIndex));
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

bool UEquipmentSlotWidget::SetLinkedSlotItem(TWeakObjectPtr<UInventoryItem> NewItem)
{
	if (NewItem.IsValid())
	{
		return OnEquipmentDropInSlot.Execute(NewItem->GetEquipmentItemClass(), SlotIndexInComponent);
	}

	LinkedInventoryItem.Reset();
	OnEquipmentRemoveFromSlot.ExecuteIfBound(SlotIndexInComponent);
	UpdateView();

	return false;
}

FReply UEquipmentSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!LinkedInventoryItem.IsValid())
	{
		return FReply::Handled();
	}

	FKey MouseBtn = InMouseEvent.GetEffectingButton();
	if (MouseBtn == EKeys::RightMouseButton)
	{
		/* Some simplification, so as not to complicate the architecture
		 * - on instancing item, we use the current pawn as an outer one.
		 * In real practice we need use callback for inform item holder what action was do in UI */

		APawn* ItemOwner = Cast<APawn>(LinkedInventoryItem->GetOuter());
		LinkedInventoryItem->RemoveFromEquipment(ItemOwner, SlotIndexInComponent);

		return FReply::Handled();
	}

	FEventReply Reply = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton);
	return Reply.NativeReply;
}

void UEquipmentSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	checkf(DragAndDropWidgetClass.Get() != nullptr, TEXT("UEquipmentSlotWidget::NativeOnDragDetected drag and drop widget is not defined"));

	UDragDropOperation* DragOperation = Cast<UDragDropOperation>(UWidgetBlueprintLibrary::CreateDragDropOperation(UDragDropOperation::StaticClass()));

	/* Some simplification for not define new widget for drag and drop operation  */
	UInventorySlotWidget* DragWidget = CreateWidget<UInventorySlotWidget>(GetOwningPlayer(), DragAndDropWidgetClass);
	DragWidget->SetItemIcon(LinkedInventoryItem->GetDescription().Icon);

	DragOperation->DefaultDragVisual = DragWidget;
	DragOperation->Pivot = EDragPivot::CenterCenter;
	LinkedInventoryItem->SetPreviousEquipmentSlotWidget(this);
	DragOperation->Payload = LinkedInventoryItem.Get();
	OutOperation = DragOperation;

	SetLinkedSlotItem(nullptr);
}

bool UEquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	bool Result = false;
	const auto NewItem = TWeakObjectPtr<UInventoryItem>(Cast<UInventoryItem>(InOperation->Payload));
	if (NewItem.IsValid())
	{
		if (!NewItem->IsEquipment())
		{
			return false;
		}

		const auto CachedLinkedInventoryItem = LinkedInventoryItem;
		Result = OnEquipmentDropInSlot.Execute(NewItem->GetEquipmentItemClass(), SlotIndexInComponent);
		if (Result && CachedLinkedInventoryItem.IsValid())
		{
			UInventorySlotWidget* PreviousInventoryWidget = NewItem->GetPreviousInventorySlotWidget();
			if (IsValid(PreviousInventoryWidget))
			{
				PreviousInventoryWidget->SetLinkedSlotItem(CachedLinkedInventoryItem);
			}
			else
			{
				UEquipmentSlotWidget* PreviousEquipmentWidget = NewItem->GetPreviousEquipmentSlotWidget();
				if (IsValid(PreviousEquipmentWidget))
				{
					PreviousEquipmentWidget->SetLinkedSlotItem(CachedLinkedInventoryItem);
				}
			}
		}
	}
	return Result;
}

void UEquipmentSlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	SetLinkedSlotItem(Cast<UInventoryItem>(InOperation->Payload));
}