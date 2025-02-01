// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItem.h"

void UInventoryItem::Initialize(EInventoryItemType ItemType_In, const FInventoryItemDescription& Description_In)
{
	ItemType = ItemType_In;
	Description.Icon = Description_In.Icon;
	Description.Name = Description_In.Name;
	bIsInitialized = true;
}
