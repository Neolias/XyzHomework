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
		AttributesComponent->SetCurrentStamina(AttributesComponent->GetMaxStamina());
		BaseCharacter->GetCharacterInventoryComponent()->RemoveInventoryItem(ItemType, 1);
		return true;
	}

	return false;
}
