// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItem.h"

#include "Actors/Equipment/EquipmentItem.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "Components/CharacterComponents/CharacterInventoryComponent.h"
#include "UI/Widgets/Equipment/EquipmentSlotWidget.h"
#include "UI/Widgets/Equipment/EquipmentViewWidget.h"
#include "UI/Widgets/Inventory/InventorySlotWidget.h"

void UInventoryItem::Initialize(EInventoryItemType ItemType_In, const FInventoryItemDescription& Description_In, TSubclassOf<AEquipmentItem> EquipmentItemClass_In)
{
	ItemType = ItemType_In;
	Description.Icon = Description_In.Icon;
	Description.Name = Description_In.Name;
	bCanStackItems = Description_In.bCanStackItems;
	EquipmentItemClass = EquipmentItemClass_In;
	bIsEquipment = IsValid(EquipmentItemClass_In);
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

	bool Result = false;
	const AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(Pawn);
	if (IsValid(BaseCharacter))
	{
		Result = BaseCharacter->GetCharacterEquipmentComponent()->AddEquipmentItem(EquipmentItemClass);

		if (Result)
		{
			if (IsValid(PreviousInventorySlotWidget))
			{
				BaseCharacter->GetCharacterInventoryComponent()->RemoveInventoryItem(PreviousInventorySlotWidget->GetLinkedSlot(), Count);
			}
			else
			{
				BaseCharacter->GetCharacterInventoryComponent()->RemoveInventoryItem(ItemType, Count);
			}
		}
	}

	return Result;
}

bool UInventoryItem::RemoveFromEquipment(APawn* Pawn, int32 EquipmentSlotIndex)
{
	bool Result = false;
	const AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(Pawn);
	if (IsValid(BaseCharacter))
	{
		Result = BaseCharacter->GetCharacterInventoryComponent()->AddInventoryItem(ItemType, Count, BaseCharacter->GetInventoryItemDataTable());
		if (Result)
		{
			BaseCharacter->GetCharacterEquipmentComponent()->RemoveEquipmentItem(EquipmentSlotIndex);
		}
	}

	return Result;
}
