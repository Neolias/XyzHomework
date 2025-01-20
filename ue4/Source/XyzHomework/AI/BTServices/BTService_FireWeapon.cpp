// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTServices/BTService_FireWeapon.h"

#include "AIController.h"
#include "XyzHomeworkTypes.h"
#include "Actors/Equipment/Weapons/RangedWeaponItem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

UBTService_FireWeapon::UBTService_FireWeapon()
{
	NodeName = "FireWeapon";
}

void UBTService_FireWeapon::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, const float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	const AController* AIController = OwnerComp.GetAIOwner();
	if (!IsValid(AIController))
	{
		return;
	}

	AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(AIController->GetPawn());
	if (!IsValid(BaseCharacter))
	{
		return;
	}

	const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!IsValid(Blackboard))
	{
		return;
	}

	const UCharacterEquipmentComponent* EquipmentComponent = BaseCharacter->GetCharacterEquipmentComponent();
	if (!IsValid(EquipmentComponent))
	{
		return;
	}

	const ARangedWeaponItem* RangedWeapon = EquipmentComponent->GetCurrentRangedWeapon();
	if (!IsValid(RangedWeapon))
	{
		return;
	}

	const AActor* CurrentTarget = Cast<AActor>(Blackboard->GetValueAsObject(AICharacterBTCurrentTargetName));
	const bool bCanSeeTarget = IsValid(CurrentTarget) && Blackboard->GetValueAsBool(AICharacterBTCanSeeTargetName);
	const bool bIsAiming = BaseCharacter->IsAiming();
	const bool bIsFiring = RangedWeapon->IsFiring();

	if (bCanSeeTarget)
	{
		// Can fire weapon only if aiming
		if (bIsAiming && !bIsFiring && !RangedWeapon->IsReloading())
		{
			BaseCharacter->StartWeaponFire();
		}
	}
	else if (bIsFiring)
	{
		BaseCharacter->StopWeaponFire();
	}
}
