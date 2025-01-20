// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTasks/BTTask_ThrowItem.h"

#include "AIController.h"
#include "XyzHomeworkTypes.h"
#include "Actors/Equipment/Throwables/ThrowableItem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

UBTTask_ThrowItem::UBTTask_ThrowItem()
{
	NodeName = "ThrowItem";
}

EBTNodeResult::Type UBTTask_ThrowItem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AController* AIController = OwnerComp.GetAIOwner();
	if (!IsValid(AIController))
	{
		return EBTNodeResult::Failed;
	}

	AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(AIController->GetPawn());
	if (!IsValid(BaseCharacter))
	{
		return EBTNodeResult::Failed;
	}

	const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!IsValid(Blackboard))
	{
		return EBTNodeResult::Failed;
	}

	const AActor* CurrentTarget = Cast<AActor>(Blackboard->GetValueAsObject(AICharacterBTCurrentTargetName));
	const bool bCanSeeTarget = IsValid(CurrentTarget) && Blackboard->GetValueAsBool(AICharacterBTCanSeeTargetName);

	if (bCanSeeTarget)
	{
		const UCharacterEquipmentComponent* EquipmentComponent = BaseCharacter->GetCharacterEquipmentComponent();
		if (IsValid(EquipmentComponent))
		{
			if (!EquipmentComponent->IsPrimaryItemEquipped())
			{
				BaseCharacter->TogglePrimaryItem();
			}

			const AThrowableItem* ThrowableItem = EquipmentComponent->GetCurrentThrowableItem();
			if (IsValid(ThrowableItem))
			{
				if (!ThrowableItem->IsThrowing())
				{
					BaseCharacter->ThrowItem();
					return EBTNodeResult::Succeeded;
				}
			}
		}
	}

	return EBTNodeResult::Failed;
}
