// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Animations/Notifies/AnimNotify_LaunchThrowableItem.h"

#include "Actors/Equipment/Throwables/ThrowableItem.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

void UAnimNotify_LaunchThrowableItem::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	const AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(MeshComp->GetOwner());
	if (IsValid(BaseCharacter))
	{
		AThrowableItem* ThrowableItem = BaseCharacter->GetCharacterEquipmentComponent()->GetCurrentThrowableItem();
		if (IsValid(ThrowableItem))
		{
			ThrowableItem->LaunchProjectile();
		}
	}
}
