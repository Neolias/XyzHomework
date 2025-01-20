// Fill out your copyright notice in the Description page of Project Settings.


#include "ReticleWidget.h"

#include "Actors/Equipment/EquipmentItem.h"

void UReticleWidget::OnAimingStateChanged_Implementation(const bool bIsAiming)
{
	CurrentReticleType = CurrentEquippedItem.IsValid() ? (bIsAiming ? CurrentEquippedItem->GetAimingReticleType() : CurrentEquippedItem->GetReticleType()) : EReticleType::None;
}

void UReticleWidget::OnEquippedItemChanged_Implementation(const AEquipmentItem* EquippedItem)
{
	CurrentEquippedItem = EquippedItem;
	CurrentReticleType = CurrentEquippedItem.IsValid() ? EquippedItem->GetReticleType() : EReticleType::None;
}
