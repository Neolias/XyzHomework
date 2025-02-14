// Fill out your copyright notice in the Description page of Project Settings.


#include "AdrenalineInventoryItem.h"

#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterAttributesComponent.h"
#include "Components/CharacterComponents/CharacterInventoryComponent.h"

bool UAdrenalineInventoryItem::Consume(APawn* Pawn)
{
	Super::Consume(Pawn);

	const AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(Pawn);
	if (IsValid(BaseCharacter))
	{
		UCharacterAttributesComponent* AttributesComponent = BaseCharacter->GetCharacterAttributesComponent();

		if (BaseCharacter->GetCharacterInventoryComponent()->RemoveInventoryItem(Description.InventoryItemType, 1))
		{
			AttributesComponent->SetCurrentStamina(AttributesComponent->GetMaxStamina());
			return true;
		}
	}

	return false;
}
