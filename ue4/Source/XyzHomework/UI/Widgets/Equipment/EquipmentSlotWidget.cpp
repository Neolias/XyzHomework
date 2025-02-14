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
		ItemIcon->SetBrushFromTexture(LinkedInventoryItem->GetItemDescription().Icon);
		ItemName->SetText(LinkedInventoryItem->GetItemDescription().Name);
	}
	else
	{
		ItemIcon->SetBrushFromTexture(nullptr);
		ItemName->SetText(FText::FromName(NAME_None));
	}

	OnEquipmentSlotUpdated.ExecuteIfBound(SlotIndexInComponent);
}

bool UEquipmentSlotWidget::SetLinkedSlotItem(TWeakObjectPtr<UInventoryItem> NewItem)
{
	const bool Result = NewItem.IsValid() ? OnEquipmentDropInSlot.Execute(NewItem->GetEquipmentItemClass(), NewItem->GetCount(), SlotIndexInComponent)
		: OnEquipmentRemoveFromSlot.Execute(SlotIndexInComponent);

	if (Result && LinkedInventoryItem.IsValid())
	{
		LinkedInventoryItem->SetPreviousEquipmentSlotWidget(this);
	}
	return Result;
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

		LinkedInventoryItem->SetPreviousEquipmentSlotWidget(this);
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
	DragWidget->UpdateItemIconAndCount(LinkedInventoryItem);

	DragOperation->DefaultDragVisual = DragWidget;
	DragOperation->Pivot = EDragPivot::CenterCenter;
	LinkedInventoryItem->SetPreviousEquipmentSlotWidget(this);
	DragOperation->Payload = LinkedInventoryItem.Get();
	OutOperation = DragOperation;

	SetLinkedSlotItem(nullptr);
}

bool UEquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	const auto PayloadItem = TWeakObjectPtr<UInventoryItem>(Cast<UInventoryItem>(InOperation->Payload));
	if (!PayloadItem.IsValid() || !PayloadItem->IsEquipment())
	{
		return false;
	}

	if (!LinkedInventoryItem.IsValid())
	{
		return SetLinkedSlotItem(PayloadItem);
	}

	const bool bCanStackItems = LinkedInventoryItem->GetInventoryItemType() == PayloadItem->GetInventoryItemType() && LinkedInventoryItem->GetAvailableSpaceInStack() > 0;

	return bCanStackItems ? StackSlotItems(PayloadItem) : SwapSlotItems(PayloadItem);
}

void UEquipmentSlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	SetLinkedSlotItem(Cast<UInventoryItem>(InOperation->Payload));
}

bool UEquipmentSlotWidget::StackSlotItems(TWeakObjectPtr<UInventoryItem> OtherSlotItem)
{
	if (!LinkedInventoryItem.IsValid() || !OtherSlotItem.IsValid())
	{
		return false;
	}

	bool Result = false;
	if (LinkedInventoryItem->CanStackItems() && OtherSlotItem->CanStackItems())
	{
		const int32 CurrentCount = LinkedInventoryItem->GetCount();
		const int32 PayloadCount = OtherSlotItem->GetCount();
		int32 Remainder = OtherSlotItem->GetCount();
		Remainder -= LinkedInventoryItem->AddCount(Remainder);
		if (Remainder)
		{
			OtherSlotItem->SetCount(Remainder);
			Result = UpdatePreviousSlot(OtherSlotItem, OtherSlotItem);
		}
		else
		{
			Result = UpdatePreviousSlot(OtherSlotItem, nullptr);
		}

		if (!Result)
		{
			LinkedInventoryItem->SetCount(CurrentCount);
			OtherSlotItem->SetCount(PayloadCount);
		}

		UpdateView();
	}

	return Result;
}

bool UEquipmentSlotWidget::SwapSlotItems(TWeakObjectPtr<UInventoryItem> OtherSlotItem)
{
	if (!OtherSlotItem.IsValid())
	{
		return false;
	}

	if (UpdatePreviousSlot(OtherSlotItem, LinkedInventoryItem))
	{
		if (SetLinkedSlotItem(OtherSlotItem))
		{
			return true;
		}

		UpdatePreviousSlot(OtherSlotItem, OtherSlotItem);
	}

	return false;
}

bool UEquipmentSlotWidget::UpdatePreviousSlot(TWeakObjectPtr<UInventoryItem> SlotReference, TWeakObjectPtr<UInventoryItem> NewSlotItem)
{
	UInventorySlotWidget* PreviousInventoryWidget = SlotReference->GetPreviousInventorySlotWidget();
	if (IsValid(PreviousInventoryWidget))
	{
		PreviousInventoryWidget->SetLinkedSlotItem(NewSlotItem);
		return true;
	}

	// Use the code below to make items swap [when dragging] within the equipment view widget
	// Note that this causes a bug which adds duplicates to the inventory view widget
	// Fix: avoid removing from equipment within UCharacterEquipmentComponent::AddEquipmentItem()

	//UEquipmentSlotWidget* PreviousEquipmentWidget = SlotReference->GetPreviousEquipmentSlotWidget();
	//return IsValid(PreviousEquipmentWidget) && PreviousEquipmentWidget->SetLinkedSlotItem(NewSlotItem);

	return true;
}
