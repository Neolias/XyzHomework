// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterInventoryComponent.h"

#include "XyzGenericEnums.h"
#include "Inventory/Items/InventoryItem.h"
#include "UI/Widgets/Inventory/InventoryViewWidget.h"

void FInventorySlot::BindOnInventorySlotUpdate(const FInventorySlotUpdate& Callback) const
{
	OnInventorySlotUpdate = Callback;
}

void FInventorySlot::UnbindOnInventorySlotUpdate()
{
	OnInventorySlotUpdate.Unbind();
}

void FInventorySlot::UpdateSlotState()
{
	OnInventorySlotUpdate.ExecuteIfBound();
}

void FInventorySlot::ClearSlot()
{
	Item = nullptr;
	UpdateSlotState();
}

UCharacterInventoryComponent::UCharacterInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UCharacterInventoryComponent::OpenViewInventory(APlayerController* PlayerController)
{
	if (!IsValid(InventoryViewWidget))
	{
		CreateViewWidget(PlayerController);
	}

	if (!InventoryViewWidget->IsVisible())
	{
		InventoryViewWidget->AddToViewport();
	}
}

void UCharacterInventoryComponent::CloseViewInventory()
{
	if (InventoryViewWidget->IsVisible())
	{
		InventoryViewWidget->RemoveFromParent();
	}
}

bool UCharacterInventoryComponent::IsViewVisible() const
{
	if (IsValid(InventoryViewWidget))
	{
		return InventoryViewWidget->IsVisible();
	}
	return false;
}

bool UCharacterInventoryComponent::AddInventoryItem(EInventoryItemType ItemType, int32 Amount)
{
	if (Amount < 1 || UsedSlotCount >= Capacity)
	{
		return false;
	}

	FInventorySlot* ItemSlot = ItemSlots.FindByPredicate([=](const FInventorySlot& Slot) {return Slot.Item.IsValid() && Slot.Item->GetItemType() == ItemType; });
	if (ItemSlot)
	{
		ItemSlot->Item->SetCount(ItemSlot->Item->GetCount() + Amount);
		ItemSlot->UpdateSlotState();
		return true;
	}

	if (IsValid(ItemDataTable))
	{
		FString RowID = UEnum::GetDisplayValueAsText<EInventoryItemType>(ItemType).ToString();
		const FItemTableRow* ItemData = ItemDataTable->FindRow<FItemTableRow>(FName(RowID), TEXT("Find item data"));

		if (ItemData)
		{
			FInventorySlot* FreeSlot = ItemSlots.FindByPredicate([=](const FInventorySlot& Slot) {return !Slot.Item.IsValid(); });
			if (FreeSlot)
			{
				const TWeakObjectPtr<UInventoryItem> NewItem = NewObject<UInventoryItem>(GetOwner(), ItemData->InventoryItemClass);
				NewItem->Initialize(ItemType, ItemData->InventoryItemDescription);
				NewItem->SetCount(NewItem->GetCount() + Amount);
				FreeSlot->Item = NewItem;
				FreeSlot->UpdateSlotState();
				UsedSlotCount++;
				return true;
			}
		}
	}

	return false;
}

void UCharacterInventoryComponent::RemoveInventoryItem(EInventoryItemType ItemType, int32 Amount)
{
	if (Amount < 1)
	{
		return;
	}

	FInventorySlot* ItemSlot = ItemSlots.FindByPredicate([=](const FInventorySlot& Slot) {return Slot.Item.IsValid() && Slot.Item->GetItemType() == ItemType; });
	if (ItemSlot)
	{
		const TWeakObjectPtr<UInventoryItem> Item = ItemSlot->Item;
		if (Item->GetCount() > Amount)
		{
			Item->SetCount(Item->GetCount() - Amount);
		}
		else
		{
			ItemSlot->ClearSlot();
			UsedSlotCount--;
		}
	}

	ItemSlot->UpdateSlotState();
}

void UCharacterInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	ItemSlots.AddDefaulted(Capacity);
}

void UCharacterInventoryComponent::CreateViewWidget(APlayerController* PlayerController)
{
	if (IsValid(InventoryViewWidget))
	{
		return;
	}

	if (!IsValid(PlayerController) || !IsValid(InventoryViewWidgetClass))
	{
		return;
	}

	InventoryViewWidget = CreateWidget<UInventoryViewWidget>(PlayerController, InventoryViewWidgetClass);
	InventoryViewWidget->InitializeViewWidget(ItemSlots);
}
