// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupWeapon.h"

#include "Actors/Equipment/Weapons/RangedWeaponItem.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

void APickupWeapon::Interact(APawn* InteractingPawn)
{
	if (IsValid(RangedWeaponClass))
	{
		AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(InteractingPawn);
		if (IsValid(BaseCharacter))
		{
			BaseCharacter->GetCharacterEquipmentComponent()->AddEquipmentItemByClass(RangedWeaponClass);
		}
	}
	Destroy();
}
