// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTServices/BTService_AimWeapon.h"

#include "AIController.h"
#include "XyzHomeworkTypes.h"
#include "Actors/Equipment/Weapons/RangedWeaponItem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

UBTService_AimWeapon::UBTService_AimWeapon()
{
	NodeName = "AimWeapon";
}

void UBTService_AimWeapon::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, const float DeltaSeconds)
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
	const bool bIsReloadingWeapon = IsValid(RangedWeapon) ? RangedWeapon->IsReloading() : false;

	const float DistanceToTarget = Blackboard->GetValueAsFloat(AICharacterBTDistanceToTargetName);
	const bool bCanAim = IsValid(RangedWeapon) && DistanceToTarget > MinAimingRange && DistanceToTarget <= MaxAimingRange;
	const AActor* CurrentTarget = Cast<AActor>(Blackboard->GetValueAsObject(AICharacterBTCurrentTargetName));
	const bool bCanSeeTarget = IsValid(CurrentTarget) && Blackboard->GetValueAsBool(AICharacterBTCanSeeTargetName);
	BaseCharacter->bUseControllerRotationYaw = bCanSeeTarget;
	const bool bIsAiming = BaseCharacter->IsAiming();

	if (!bIsReloadingWeapon && bCanAim && bCanSeeTarget)
	{
		if(!bIsAiming)
		{
			BaseCharacter->SetWantsToAim();
		}
	}
	else if (bIsAiming)
	{
		BaseCharacter->ResetWantsToAim();
	}
}
