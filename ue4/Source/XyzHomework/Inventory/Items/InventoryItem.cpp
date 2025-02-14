// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItem.h"

#include "Actors/Equipment/EquipmentItem.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "Components/CharacterComponents/CharacterInventoryComponent.h"
#include "UI/Widgets/Equipment/EquipmentSlotWidget.h"
#include "UI/Widgets/Equipment/EquipmentViewWidget.h"
#include "UI/Widgets/Inventory/InventorySlotWidget.h"

void UInventoryItem::InitializeItem(const FInventoryItemDescription& InventoryItemDescription)
{
	Description = InventoryItemDescription;
	bIsEquipment = IsValid(Description.EquipmentItemClass);
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
	if (!IsEquipment() || !IsValid(Description.EquipmentItemClass))
	{
		return false;
	}

	const AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(Pawn);
	if (IsValid(BaseCharacter))
	{
		if (BaseCharacter->GetCharacterEquipmentComponent()->AddEquipmentItem(Description.EquipmentItemClass, Count))
		{
			if (IsValid(PreviousInventorySlotWidget))
			{
				BaseCharacter->GetCharacterInventoryComponent()->RemoveInventoryItem(PreviousInventorySlotWidget->GetLinkedSlot(), Count);
			}
			else
			{
				BaseCharacter->GetCharacterInventoryComponent()->RemoveInventoryItem(Description.InventoryItemType, Count);
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
		if (BaseCharacter->GetCharacterInventoryComponent()->AddInventoryItem(Description.InventoryItemType, Count))
		{
			if (BaseCharacter->GetCharacterEquipmentComponent()->RemoveEquipmentItem(EquipmentSlotIndex))
			{
				return true;
			}

			BaseCharacter->GetCharacterInventoryComponent()->RemoveInventoryItem(Description.InventoryItemType, Count);
		}
	}

	return false;
}
