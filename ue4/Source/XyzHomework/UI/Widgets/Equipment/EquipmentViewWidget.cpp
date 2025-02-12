// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentViewWidget.h"

#include "EquipmentSlotWidget.h"
#include "Actors/Equipment/EquipmentItem.h"
#include "Components/GridPanel.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

UEquipmentSlotWidget* UEquipmentViewWidget::GetEquipmentSlotWidget(int32 SlotIndex) const
{
	if (!IsValid(ItemSlots) || SlotIndex < 1 || SlotIndex - 1 >= ItemSlots->GetChildrenCount())
	{
		return nullptr;
	}

	return Cast<UEquipmentSlotWidget>(ItemSlots->GetChildAt(SlotIndex - 1));
}

void UEquipmentViewWidget::InitializeWidget(UCharacterEquipmentComponent* EquipmentComponent)
{
	CachedEquipmentComponent = EquipmentComponent;
	const TArray<AEquipmentItem*>& Items = CachedEquipmentComponent->GetEquippedItems();
	/* We skip "none" slot*/
	for (int32 Index = 1; Index < Items.Num(); ++Index)
	{
		AddSlotToView(Items[Index], Index);
	}
}

void UEquipmentViewWidget::AddSlotToView(AEquipmentItem* EquipmentItem, int32 SlotIndex)
{
	checkf(IsValid(DefaultSlotViewClass.Get()), TEXT("UEquipmentViewWidget::AddEquipmentSlotView equipment slot widget is not set"));

	UEquipmentSlotWidget* SlotWidget = CreateWidget<UEquipmentSlotWidget>(this, DefaultSlotViewClass);
	if (IsValid(SlotWidget))
	{
		if (IsValid(EquipmentItem))
		{
			SlotWidget->InitializeSlot(EquipmentItem->GetLinkedInventoryItem(), SlotIndex);
			SlotWidget->OnEquipmentDropInSlot.BindUObject(this, &UEquipmentViewWidget::EquipItem);
			SlotWidget->OnEquipmentRemoveFromSlot.BindUObject(this, &UEquipmentViewWidget::UnequipItem);
			SlotWidget->OnEquipmentSlotUpdated.BindUObject(this, &UEquipmentViewWidget::UpdateEquipment);
		}

		// Use the code below to hide empty equipment slots

		//else
		//{
		//	SlotWidget->SetVisibility(ESlateVisibility::Collapsed);
		//}
		ItemSlots->AddChildToGrid(SlotWidget, ItemSlots->GetChildrenCount(), 1);
		SlotWidget->UpdateView();
	}
}

void UEquipmentViewWidget::UpdateSlot(int32 SlotIndex)
{
	UEquipmentSlotWidget* WidgetToUpdate = GetEquipmentSlotWidget(SlotIndex);
	if (IsValid(WidgetToUpdate))
	{
		const AEquipmentItem* EquipmentItem = CachedEquipmentComponent->GetEquippedItems()[SlotIndex];
		const auto InventoryItem = IsValid(EquipmentItem) ? EquipmentItem->GetLinkedInventoryItem() : nullptr;
		WidgetToUpdate->InitializeSlot(InventoryItem, SlotIndex);
		WidgetToUpdate->UpdateView();
	}
}

bool UEquipmentViewWidget::EquipItem(const TSubclassOf<AEquipmentItem>& WeaponClass, int32 Amount, int32 SlotIndex)
{
	return CachedEquipmentComponent->AddEquipmentItem(WeaponClass, Amount, SlotIndex);
}

bool UEquipmentViewWidget::UnequipItem(int32 SlotIndex)
{
	return CachedEquipmentComponent->RemoveEquipmentItem(SlotIndex);
}

void UEquipmentViewWidget::UpdateEquipment(int32 SlotIndex)
{
	CachedEquipmentComponent->OnEquipmentSlotUpdated(SlotIndex);
}
