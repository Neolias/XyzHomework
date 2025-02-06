// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItem.h"

#include "Actors/Equipment/EquipmentItem.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

void UInventoryItem::Initialize(EInventoryItemType ItemType_In, const FInventoryItemDescription& Description_In, TSubclassOf<AEquipmentItem> EquipmentItemClass_In)
{
	ItemType = ItemType_In;
	EquipmentItemClass = EquipmentItemClass_In;
	Description.Icon = Description_In.Icon;
	Description.Name = Description_In.Name;
	bIsInitialized = true;
}

bool UInventoryItem::AddToEquipment(AXyzBaseCharacter* BaseCharacter, EEquipmentItemSlot EquipmentItemSlot)
{
	if (!IsValid(EquipmentItemClass))
	{
		return false;
	}

	if (IsValid(BaseCharacter))
	{
		return BaseCharacter->GetCharacterEquipmentComponent()->AddEquipmentItem(EquipmentItemClass, (uint32)EquipmentItemSlot);
	}

	return false;
}

void UInventoryItem::RemoveFromEquipment(class AXyzBaseCharacter* BaseCharacter, EEquipmentItemSlot EquipmentItemSlot)
{
	if (IsValid(BaseCharacter))
	{
		return BaseCharacter->GetCharacterEquipmentComponent()->RemoveEquipmentItem((uint32)EquipmentItemSlot);
	}
}
