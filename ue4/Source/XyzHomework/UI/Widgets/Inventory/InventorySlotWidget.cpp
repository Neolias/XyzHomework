// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySlotWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CharacterComponents/CharacterInventoryComponent.h"
#include "Inventory/Items/InventoryItem.h"
#include "UI/Widgets/Equipment/EquipmentSlotWidget.h"


void UInventorySlotWidget::InitializeSlot(FInventorySlot& InventorySlot)
{
	LinkedSlot = &InventorySlot;

	FInventorySlot::FInventorySlotUpdate OnInventorySlotUpdate;
	OnInventorySlotUpdate.BindUObject(this, &UInventorySlotWidget::UpdateView);
	LinkedSlot->BindOnInventorySlotUpdate(OnInventorySlotUpdate);
}

void UInventorySlotWidget::UpdateView()
{
	if (!LinkedSlot)
	{
		ItemIcon->SetBrushFromTexture(nullptr);
		ItemCount->SetText(FText::GetEmpty());
		return;
	}

	if (LinkedSlot->Item.IsValid())
	{
		ItemIcon->SetBrushFromTexture(LinkedSlot->Item->GetItemDescription().Icon);
		if (LinkedSlot->Item->CanStackItems())
		{
			const FText CountText = FText::FromString(FString::FromInt(LinkedSlot->Item->GetCount()));
			ItemCount->SetText(CountText);
		}
		else
		{
			ItemCount->SetText(FText::GetEmpty());
		}
	}
	else
	{
		ItemIcon->SetBrushFromTexture(nullptr);
		ItemCount->SetText(FText::GetEmpty());
	}
}

void UInventorySlotWidget::UpdateItemIconAndCount(TWeakObjectPtr<UInventoryItem> NewItemData)
{
	if (!NewItemData.IsValid())
	{
		UpdateView();
	}

	ItemIcon->SetBrushFromTexture(NewItemData->GetItemDescription().Icon);
	if (NewItemData->CanStackItems())
	{
		const FText CountText = FText::FromString(FString::FromInt(NewItemData->GetCount()));
		ItemCount->SetText(CountText);
	}
	else
	{
		ItemCount->SetText(FText::GetEmpty());
	}
}

FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!LinkedSlot)
	{
		return FReply::Handled();
	}

	if (!LinkedSlot->Item.IsValid())
	{
		return FReply::Handled();
	}

	FKey MouseBtn = InMouseEvent.GetEffectingButton();
	if (MouseBtn == EKeys::RightMouseButton)
	{
		/* Some simplification, so as not to complicate the architecture
		 * - on instancing item, we use the current pawn as an outer one.
		 * In real practice we need use callback for inform item holder what action was do in UI */

		TWeakObjectPtr<UInventoryItem> LinkedSlotItem = LinkedSlot->Item;
		LinkedSlotItem->SetPreviousInventorySlotWidget(this);
		APawn* ItemOwner = Cast<APawn>(LinkedSlotItem->GetOuter());

		if (LinkedSlotItem->IsEquipment())
		{
			LinkedSlotItem->AddToEquipment(ItemOwner);
		}
		else
		{
			LinkedSlotItem->Consume(ItemOwner);
		}

		return FReply::Handled();
	}

	FEventReply Reply = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton);
	return Reply.NativeReply;
}

void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	UDragDropOperation* DragOperation = Cast<UDragDropOperation>(UWidgetBlueprintLibrary::CreateDragDropOperation(UDragDropOperation::StaticClass()));

	/* Some simplification for not define new widget for drag and drop operation  */
	UInventorySlotWidget* DragWidget = CreateWidget<UInventorySlotWidget>(GetOwningPlayer(), GetClass());
	DragWidget->UpdateItemIconAndCount(LinkedSlot->Item);

	DragOperation->DefaultDragVisual = DragWidget;
	DragOperation->Pivot = EDragPivot::MouseDown;
	LinkedSlot->Item->SetPreviousInventorySlotWidget(this);
	DragOperation->Payload = LinkedSlot->Item.Get();
	OutOperation = DragOperation;

	LinkedSlot->ClearSlot();
}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	const auto PayloadItem = TWeakObjectPtr<UInventoryItem>(Cast<UInventoryItem>(InOperation->Payload));
	if (!PayloadItem.IsValid())
	{
		return false;
	}

	if (!LinkedSlot->Item.IsValid())
	{
		SetLinkedSlotItem(PayloadItem);
		return true;
	}

	const bool bCanStackItems = LinkedSlot->Item->GetInventoryItemType() == PayloadItem->GetInventoryItemType() && LinkedSlot->Item->GetAvailableSpaceInStack() > 0;

	return bCanStackItems ? StackSlotItems(PayloadItem) : SwapSlotItems(PayloadItem);
}

void UInventorySlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	SetLinkedSlotItem(Cast<UInventoryItem>(InOperation->Payload));
}

bool UInventorySlotWidget::StackSlotItems(TWeakObjectPtr<UInventoryItem> OtherSlotItem)
{
	if (!LinkedSlot->Item.IsValid() || !OtherSlotItem.IsValid())
	{
		return false;
	}

	bool Result = false;
	if (LinkedSlot->Item->CanStackItems() && OtherSlotItem->CanStackItems())
	{
		const int32 CurrentCount = LinkedSlot->Item->GetCount();
		const int32 PayloadCount = OtherSlotItem->GetCount();
		int32 Remainder = OtherSlotItem->GetCount();
		Remainder -= LinkedSlot->Item->AddCount(Remainder);
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
			LinkedSlot->Item->SetCount(CurrentCount);
			OtherSlotItem->SetCount(PayloadCount);
		}

		UpdateView();
	}

	return Result;
}

bool UInventorySlotWidget::SwapSlotItems(TWeakObjectPtr<UInventoryItem> OtherSlotItem)
{
	if (!OtherSlotItem.IsValid())
	{
		return false;
	}

	if (UpdatePreviousSlot(OtherSlotItem, LinkedSlot->Item))
	{
		SetLinkedSlotItem(OtherSlotItem);
		return true;
	}

	return false;
}

bool UInventorySlotWidget::UpdatePreviousSlot(TWeakObjectPtr<UInventoryItem> SlotReference, TWeakObjectPtr<UInventoryItem> NewSlotItem)
{
	UInventorySlotWidget* PreviousInventoryWidget = SlotReference->GetPreviousInventorySlotWidget();
	if (IsValid(PreviousInventoryWidget))
	{
		PreviousInventoryWidget->SetLinkedSlotItem(NewSlotItem);
		return true;
	}

	UEquipmentSlotWidget* PreviousEquipmentWidget = SlotReference->GetPreviousEquipmentSlotWidget();
	return IsValid(PreviousEquipmentWidget) && PreviousEquipmentWidget->SetLinkedSlotItem(NewSlotItem);
}

void UInventorySlotWidget::SetItemIcon(UTexture2D* Icon)
{
	ItemIcon->SetBrushFromTexture(Icon);
}

void UInventorySlotWidget::SetLinkedSlotItem(TWeakObjectPtr<UInventoryItem> NewItem)
{
	LinkedSlot->Item = NewItem.IsValid() && NewItem->GetCount() > 0 ? NewItem : nullptr;

	if (LinkedSlot->Item.IsValid())
	{
		LinkedSlot->Item->SetPreviousInventorySlotWidget(this);
	}
	LinkedSlot->UpdateSlotState();
}
