// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPackInventoryItem.h"

#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterAttributesComponent.h"
#include "Components/CharacterComponents/CharacterInventoryComponent.h"

bool UHealthPackInventoryItem::Consume(APawn* Pawn)
{
	Super::Consume(Pawn);

	const AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(Pawn);
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->GetCharacterAttributesComponent()->AddHealth(HealthRestoreAmount);
		BaseCharacter->GetCharacterInventoryComponent()->RemoveInventoryItem(ItemType, 1);
		return true;
	}

	return false;
}
