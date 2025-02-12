// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItem.h"

#include "Actors/Equipment/EquipmentItem.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "Components/CharacterComponents/CharacterInventoryComponent.h"
#include "UI/Widgets/Equipment/EquipmentSlotWidget.h"
#include "UI/Widgets/Equipment/EquipmentViewWidget.h"
#include "UI/Widgets/Inventory/InventorySlotWidget.h"

void UInventoryItem::Initialize(EInventoryItemType ItemType_In, const FInventoryItemDescription& Description_In, TSubclassOf<AEquipmentItem> EquipmentItemClass_In/* = nullptr*/)
{
	ItemType = ItemType_In;
	Description = Description_In;
	EquipmentItemClass = EquipmentItemClass_In;
	bIsEquipment = IsValid(EquipmentItemClass_In);
}

void UInventoryItem::SetCount(const int32 NewCount)
{
	Count = FMath::Clamp(NewCount, 0, Description.MaxCount);
}

int32 UInventoryItem::AddCount(const int32 Value)
{
	const int32 OldCount = Count;
	SetCount(Count + Value);
	return Count - OldCount;
}

int32 UInventoryItem::GetAvailableSpaceInStack() const
{
	return CanStackItems() ? Description.MaxCount - Count : 0;
}

void UInventoryItem::SetPreviousInventorySlotWidget(UInventorySlotWidget* SlotWidget)
{
	PreviousInventorySlotWidget = SlotWidget;
	PreviousEquipmentSlotWidget = nullptr;
}

void UInventoryItem::SetPreviousEquipmentSlotWidget(UEquipmentSlotWidget* SlotWidget)
{
	PreviousEquipmentSlotWidget = SlotWidget;
	PreviousInventorySlotWidget = nullptr;
}

bool UInventoryItem::AddToEquipment(APawn* Pawn)
{
	if (!IsEquipment() || !IsValid(EquipmentItemClass))
	{
		return false;
	}

	const AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(Pawn);
	if (IsValid(BaseCharacter))
	{
		if (BaseCharacter->GetCharacterEquipmentComponent()->AddEquipmentItem(EquipmentItemClass, Count))
		{
			if (IsValid(PreviousInventorySlotWidget))
			{
				BaseCharacter->GetCharacterInventoryComponent()->RemoveInventoryItem(PreviousInventorySlotWidget->GetLinkedSlot(), Count);
			}
			else
			{
				BaseCharacter->GetCharacterInventoryComponent()->RemoveInventoryItem(ItemType, Count);
			}

			return true;
		}
	}

	return false;
}

bool UInventoryItem::RemoveFromEquipment(APawn* Pawn, int32 EquipmentSlotIndex)
{
	const AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(Pawn);
	if (IsValid(BaseCharacter))
	{
		if (BaseCharacter->GetCharacterInventoryComponent()->AddInventoryItem(ItemType, Count, BaseCharacter->GetInventoryItemDataTable()))
		{
			if (BaseCharacter->GetCharacterEquipmentComponent()->RemoveEquipmentItem(EquipmentSlotIndex))
			{
				return true;
			}

			BaseCharacter->GetCharacterInventoryComponent()->RemoveInventoryItem(ItemType, Count);
		}
	}

	return false;
}
