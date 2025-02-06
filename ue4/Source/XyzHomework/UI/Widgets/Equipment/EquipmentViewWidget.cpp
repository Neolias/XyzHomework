// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentViewWidget.h"

#include "EquipmentSlotWidget.h"
#include "Actors/Equipment/EquipmentItem.h"
#include "Components/VerticalBox.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

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

		SlotWidget->InitializeSlot(EquipmentItem, SlotIndex);

		ItemSlots->AddChildToVerticalBox(SlotWidget);
		SlotWidget->UpdateView();
		SlotWidget->OnEquipmentDropInSlot.BindUObject(this, &UEquipmentViewWidget::EquipItem);
		SlotWidget->OnEquipmentRemoveFromSlot.BindUObject(this, &UEquipmentViewWidget::UnequipItem);
	}
}

void UEquipmentViewWidget::UpdateSlot(int32 SlotIndex)
{
	UEquipmentSlotWidget* WidgetToUpdate = Cast<UEquipmentSlotWidget>(ItemSlots->GetChildAt(SlotIndex - 1));
	if (IsValid(WidgetToUpdate))
	{
		WidgetToUpdate->InitializeSlot(CachedEquipmentComponent->GetEquippedItems()[SlotIndex], SlotIndex);
		WidgetToUpdate->UpdateView();
	}
}

bool UEquipmentViewWidget::EquipItem(const TSubclassOf<AEquipmentItem>& WeaponClass, int32 SlotIndex)
{
	const bool Result = CachedEquipmentComponent->AddEquipmentItem(WeaponClass, SlotIndex);
	if (Result)
	{
		UpdateSlot(SlotIndex);
	}
	return Result;
}

void UEquipmentViewWidget::UnequipItem(int32 SlotIndex)
{
	CachedEquipmentComponent->RemoveEquipmentItem(SlotIndex);
	UpdateSlot(SlotIndex);
}