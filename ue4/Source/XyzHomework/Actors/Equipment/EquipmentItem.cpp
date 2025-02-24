// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Equipment/EquipmentItem.h"

#include "Characters/XyzBaseCharacter.h"
#include "Inventory/Items/InventoryItem.h"

AEquipmentItem::AEquipmentItem()
{
	SetReplicates(true);
}

void AEquipmentItem::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);

	if (IsValid(NewOwner))
	{
		checkf(GetOwner()->IsA<AXyzBaseCharacter>(), TEXT("EquipmentItem object should be owned by AXyzBaseCharacter."))
			CachedBaseCharacterOwner = StaticCast<AXyzBaseCharacter*>(GetOwner());
		if (GetLocalRole() == ROLE_Authority)
		{
			SetAutonomousProxy(true);
		}
	}
	else
	{
		CachedBaseCharacterOwner = nullptr;
	}
}

void AEquipmentItem::SetLinkedInventoryItem(const TWeakObjectPtr<UInventoryItem> InventoryItem)
{
	LinkedInventoryItem = InventoryItem;
}

bool AEquipmentItem::IsEquipmentSlotCompatible(const EEquipmentItemSlot EquipmentSlot) const
{
	return CompatibleEquipmentSlots.Contains(EquipmentSlot);
}

void AEquipmentItem::OnLevelDeserialized_Implementation()
{
	if (!IsValid(Cast<ACharacter>(GetOwner())))
	{
		Destroy();
		return;
	}

	SetActorRelativeTransform(FTransform(FRotator::ZeroRotator, FVector::ZeroVector));
}
