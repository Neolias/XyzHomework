// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySlotWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
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
		ImageItemIcon->SetBrushFromTexture(nullptr);
		return;
	}

	if (LinkedSlot->Item.IsValid())
	{
		const FInventoryItemDescription& Description = LinkedSlot->Item->GetDescription();
		ImageItemIcon->SetBrushFromTexture(Description.Icon);
	}
	else
	{
		ImageItemIcon->SetBrushFromTexture(nullptr);
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
	DragWidget->ImageItemIcon->SetBrushFromTexture(LinkedSlot->Item->GetDescription().Icon);

	DragOperation->DefaultDragVisual = DragWidget;
	DragOperation->Pivot = EDragPivot::MouseDown;
	LinkedSlot->Item->SetPreviousInventorySlotWidget(this);
	DragOperation->Payload = LinkedSlot->Item.Get();
	OutOperation = DragOperation;

	LinkedSlot->ClearSlot();
}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	const auto NewItem = TWeakObjectPtr<UInventoryItem>(Cast<UInventoryItem>(InOperation->Payload));
	if (!LinkedSlot->Item.IsValid())
	{
		SetLinkedSlotItem(NewItem);
		return true;
	}

	if (NewItem.IsValid())
	{
		if (LinkedSlot->Item->GetItemType() != NewItem->GetItemType())
		{
			UInventorySlotWidget* PreviousInventoryWidget = NewItem->GetPreviousInventorySlotWidget();
			if (IsValid(PreviousInventoryWidget))
			{
				PreviousInventoryWidget->SetLinkedSlotItem(LinkedSlot->Item);
			}
			else
			{
				UEquipmentSlotWidget* PreviousEquipmentWidget = NewItem->GetPreviousEquipmentSlotWidget();
				if (IsValid(PreviousEquipmentWidget))
				{
					if (!PreviousEquipmentWidget->SetLinkedSlotItem(LinkedSlot->Item))
					{
						return false;
					}
				}
			}

			SetLinkedSlotItem(NewItem);
			return true;
		}
	}

	return false;
}

void UInventorySlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	LinkedSlot->Item = TWeakObjectPtr<UInventoryItem>(Cast<UInventoryItem>(InOperation->Payload));
	LinkedSlot->UpdateSlotState();
}

void UInventorySlotWidget::SetItemIcon(UTexture2D* Icon)
{
	ImageItemIcon->SetBrushFromTexture(Icon);
}

void UInventorySlotWidget::SetLinkedSlotItem(TWeakObjectPtr<UInventoryItem> NewItem)
{
	if (!LinkedSlot)
	{
		return;
	}

	LinkedSlot->Item = NewItem;
	LinkedSlot->UpdateSlotState();
}
