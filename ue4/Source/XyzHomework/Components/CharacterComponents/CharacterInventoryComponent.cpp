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

void UCharacterInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	ItemSlots.AddDefaulted(Capacity);
}

void UCharacterInventoryComponent::CreateViewWidget(APlayerController* PlayerController, UDataTable* InventoryItemDataTable)
{
	CachedItemDataTable = InventoryItemDataTable;

	if (IsValid(InventoryViewWidget))
	{
		return;
	}

	if (!IsValid(PlayerController) || !IsValid(InventoryViewWidgetClass))
	{
		return;
	}

	InventoryViewWidget = CreateWidget<UInventoryViewWidget>(PlayerController, InventoryViewWidgetClass);
	InventoryViewWidget->InitializeWidget(ItemSlots);
}

void UCharacterInventoryComponent::OpenViewInventory(APlayerController* PlayerController, UDataTable* InventoryItemDataTable)
{
	CachedItemDataTable = InventoryItemDataTable;

	if (!IsValid(InventoryViewWidget))
	{
		CreateViewWidget(PlayerController, InventoryItemDataTable);
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

bool UCharacterInventoryComponent::IsViewInventoryVisible() const
{
	if (IsValid(InventoryViewWidget))
	{
		return InventoryViewWidget->IsVisible();
	}
	return false;
}

bool UCharacterInventoryComponent::AddInventoryItem(EInventoryItemType ItemType, int32 Amount, UDataTable* InventoryItemDataTable)
{
	if (Amount < 1)
	{
		return false;
	}

	CachedItemDataTable = InventoryItemDataTable;

	FInventorySlot* ItemSlot = ItemSlots.FindByPredicate([=](const FInventorySlot& Slot) {return Slot.Item.IsValid() && Slot.Item->GetItemType() == ItemType; });
	if (ItemSlot && ItemSlot->Item->CanStackItems())
	{
		ItemSlot->Item->SetCount(ItemSlot->Item->GetCount() + Amount);
		ItemSlot->UpdateSlotState();
		return true;
	}

	if (UsedSlotCount >= Capacity)
	{
		return false;
	}

	if (IsValid(CachedItemDataTable))
	{
		FString RowID = UEnum::GetDisplayValueAsText<EInventoryItemType>(ItemType).ToString();
		const FInventoryTableRow* ItemData = CachedItemDataTable->FindRow<FInventoryTableRow>(FName(RowID), TEXT("Find item data"));

		if (ItemData)
		{
			FInventorySlot* FreeSlot = ItemSlots.FindByPredicate([=](const FInventorySlot& Slot) {return !Slot.Item.IsValid(); });
			if (FreeSlot)
			{
				const TWeakObjectPtr<UInventoryItem> NewItem = NewObject<UInventoryItem>(GetOwner(), ItemData->InventoryItemClass);
				NewItem->Initialize(ItemType, ItemData->InventoryItemDescription, ItemData->EquipmentItemClass);
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

	FInventorySlot* Slot = ItemSlots.FindByPredicate([=](const FInventorySlot& Slot) {return Slot.Item.IsValid() && Slot.Item->GetItemType() == ItemType; });
	RemoveInventoryItem(Slot, Amount);
}

void UCharacterInventoryComponent::RemoveInventoryItem(int32 SlotIndex, int32 Amount)
{
	if (SlotIndex >= 0 && SlotIndex < ItemSlots.Num())
	{
		RemoveInventoryItem(&ItemSlots[SlotIndex], Amount);
	}
}

void UCharacterInventoryComponent::RemoveInventoryItem(FInventorySlot* Slot, int32 Amount)
{
	if (Slot)
	{
		const TWeakObjectPtr<UInventoryItem> Item = Slot->Item;
		if (Item->GetCount() > Amount)
		{
			Item->SetCount(Item->GetCount() - Amount);
		}
		else
		{
			Slot->ClearSlot();
			UsedSlotCount--;
		}
	}

	Slot->UpdateSlotState();
}
