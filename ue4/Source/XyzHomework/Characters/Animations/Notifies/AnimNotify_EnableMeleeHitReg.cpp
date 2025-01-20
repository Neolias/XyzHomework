// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Animations/Notifies/AnimNotify_EnableMeleeHitReg.h"

#include "Actors/Equipment/Weapons/MeleeWeaponItem.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

void UAnimNotify_EnableMeleeHitReg::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	const AXyzBaseCharacter* CharacterOwner = Cast<AXyzBaseCharacter>(MeshComp->GetOwner());
	if (IsValid(CharacterOwner))
	{
		AMeleeWeaponItem* MeleeWeaponItem = CharacterOwner->GetCharacterEquipmentComponent()->GetCurrentMeleeWeapon();
		if (IsValid(MeleeWeaponItem))
		{
			MeleeWeaponItem->EnableHitRegistration(bIsHitRegistrationEnabled);
		}
	}
}
