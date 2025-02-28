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
	InventoryViewWidget->InitializeWidget(ItemSlots);
}

void UCharacterInventoryComponent::OpenViewInventory(APlayerController* PlayerController)
{
	if (!IsValid(InventoryViewWidget))
	{
		CreateViewWidget(PlayerController);
	}

	if (IsValid(InventoryViewWidget) && !InventoryViewWidget->IsVisible())
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

bool UCharacterInventoryComponent::AddInventoryItem(EInventoryItemType ItemType, int32 Amount)
{
	if (Amount < 1)
	{
		return false;
	}

	const int32 Remainder = StackItems(ItemType, Amount);
	if (Remainder == -1) // Indicates error
	{
		return false;
	}

	return FillEmptySlots(ItemType, Remainder);
}

int32 UCharacterInventoryComponent::StackItems(EInventoryItemType ItemType, int32 Amount)
{
	if (Amount < 1)
	{
		return 0;
	}

	TArray<FInventorySlot*> CompatibleItemSlots;
	int32 MaxCountPerSlot = 0;
	int32 AvailableSpaceInSlots = 0;
	for (FInventorySlot& Slot : ItemSlots)
	{
		if (Slot.Item.IsValid() && Slot.Item->GetInventoryItemType() == ItemType && Slot.Item->CanStackItems())
		{
			CompatibleItemSlots.Add(&Slot);
			MaxCountPerSlot = Slot.Item->GetMaxCount();
			AvailableSpaceInSlots += MaxCountPerSlot - Slot.Item->GetCount();
		}
	}

	if (CompatibleItemSlots.Num() > 0 && Amount > AvailableSpaceInSlots + (Capacity - UsedSlotCount) * MaxCountPerSlot)
	{
		return -1; // indicates error
	}

	int32 Remainder = Amount;
	for (FInventorySlot* Slot : CompatibleItemSlots)
	{
		Remainder -= Slot->Item->AddCount(Remainder);
		Slot->UpdateSlotState();

		if (Remainder == 0)
		{
			break;
		}
	}
	return Remainder;
}

bool UCharacterInventoryComponent::FillEmptySlots(EInventoryItemType ItemType, int32 Amount)
{
	if (Amount < 1)
	{
		return true;
	}

	int32 Remainder = Amount;
	const int32 FreeSlotCount = Capacity - UsedSlotCount;
	if (Remainder && !FreeSlotCount)
	{
		return false;
	}

	const UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *InventoryItemDataTable.GetUniqueID().GetAssetPathString());
	if (IsValid(DataTable))
	{
		FString RowID = UEnum::GetDisplayValueAsText<EInventoryItemType>(ItemType).ToString();
		const FInventoryTableRow* ItemData = DataTable->FindRow<FInventoryTableRow>(FName(RowID), TEXT("Find item data"));

		if (ItemData)
		{
			const int32 RequiredSlotCount = FMath::CeilToInt((float)Remainder / ItemData->InventoryItemDescription.MaxCount);
			if (RequiredSlotCount > FreeSlotCount)
			{
				return false;
			}

			for (int32 i = 0; i < RequiredSlotCount; ++i)
			{
				FInventorySlot* FreeSlot = ItemSlots.FindByPredicate([=](const FInventorySlot& Slot) {return !Slot.Item.IsValid(); });
				if (FreeSlot)
				{
					const TWeakObjectPtr<UInventoryItem> NewItem = NewObject<UInventoryItem>(GetOwner(), ItemData->InventoryItemClass);
					NewItem->InitializeItem(ItemData->InventoryItemDescription);
					Remainder -= NewItem->AddCount(Remainder);
					FreeSlot->Item = NewItem;
					FreeSlot->UpdateSlotState();
					UsedSlotCount++;
				}
			}
		}
	}

	return Remainder == 0;
}

int32 UCharacterInventoryComponent::RemoveInventoryItem(EInventoryItemType ItemType, int32 Amount)
{
	if (Amount < 1)
	{
		return 0;
	}

	TArray<FInventorySlot*> CompatibleItemSlots;
	for (FInventorySlot& Slot : ItemSlots)
	{
			if (Slot.Item.IsValid() && Slot.Item->GetInventoryItemType() == ItemType && Slot.Item->CanStackItems())
			{
				CompatibleItemSlots.Add(&Slot);
			}
	}
	CompatibleItemSlots.Sort([=](const FInventorySlot& SlotA, const FInventorySlot& SlotB) { return SlotA.Item->GetCount() < SlotB.Item->GetCount(); });

	int32 Remainder = Amount;
	for (FInventorySlot* Slot : CompatibleItemSlots)
	{
		Remainder -= RemoveInventoryItem(Slot, Remainder);
		if (Remainder < 1)
		{
			break;
		}
	}

	return FMath::Clamp(Amount - Remainder, 0, Amount);
}

int32 UCharacterInventoryComponent::RemoveInventoryItem(int32 SlotIndex, int32 Amount)
{
	if (SlotIndex >= 0 && SlotIndex < ItemSlots.Num())
	{
		return RemoveInventoryItem(&ItemSlots[SlotIndex], Amount);
	}

	return 0;
}

int32 UCharacterInventoryComponent::RemoveInventoryItem(FInventorySlot* Slot, int32 Amount)
{
	if (!Slot)
	{
		return 0;
	}

	int32 Result = 0;
	if (Slot->Item.IsValid())
	{
		const int32 ItemCount = Slot->Item->GetCount();
		if (ItemCount > Amount)
		{
			Slot->Item->SetCount(ItemCount - Amount);
			Result = Amount;
		}
		else
		{
			Slot->ClearSlot();
			UsedSlotCount--;
			Result = ItemCount;
		}
	}

	Slot->UpdateSlotState();
	return Result;
}

bool UCharacterInventoryComponent::AddAmmoItem(EWeaponAmmoType AmmoType, int32 Amount)
{
	if (Amount < 1)
	{
		return true;
	}

	const UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *InventoryItemDataTable.GetUniqueID().GetAssetPathString());
	if (IsValid(DataTable))
	{
		FString RowID = UEnum::GetDisplayValueAsText<EWeaponAmmoType>(AmmoType).ToString() + FString("Ammo");
		const FInventoryTableRow* ItemData = DataTable->FindRow<FInventoryTableRow>(FName(RowID), TEXT("Find item data"));
		if (ItemData)
		{
			return AddInventoryItem(ItemData->InventoryItemDescription.InventoryItemType, Amount);
		}
	}

	return true; // If ammo data is not found, returns true to let the equipment component fill EquipmentAmmoArray
}

int32 UCharacterInventoryComponent::RemoveAmmoItem(EWeaponAmmoType AmmoType, int32 Amount)
{
	if (Amount < 1)
	{
		return 0;
	}

	const UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *InventoryItemDataTable.GetUniqueID().GetAssetPathString());
	if (IsValid(DataTable))
	{
		FString RowID = UEnum::GetDisplayValueAsText<EWeaponAmmoType>(AmmoType).ToString() + FString("Ammo");
		const FInventoryTableRow* ItemData = DataTable->FindRow<FInventoryTableRow>(FName(RowID), TEXT("Find item data"));
		if (ItemData)
		{
			return RemoveInventoryItem(ItemData->InventoryItemDescription.InventoryItemType, Amount);
		}
	}

	return Amount; // If ammo data is not found, returns requested amount to let ammo reloading proceed
}
