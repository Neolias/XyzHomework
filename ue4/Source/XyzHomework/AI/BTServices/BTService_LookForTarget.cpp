// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTServices/BTService_LookForTarget.h"

#include "AIController.h"
#include "XyzHomeworkTypes.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/PlayerCharacter.h"

UBTService_LookForTarget::UBTService_LookForTarget()
{
	NodeName = "LookForTarget";
}

void UBTService_LookForTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, const float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!IsValid(AIController))
	{
		return;
	}

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!IsValid(Blackboard))
	{
		return;
	}

	const AActor* CurrentTarget = Cast<AActor>(Blackboard->GetValueAsObject(AICharacterBTCurrentTargetName));
	if (IsValid(CurrentTarget))
	{
		Blackboard->SetValueAsBool(AICharacterBTCanSeeTargetName, CanSeeTarget(AIController));

		const float Distance = FVector::Dist(CurrentTarget->GetActorLocation(), AIController->GetPawn()->GetActorLocation());
		Blackboard->SetValueAsFloat(AICharacterBTDistanceToTargetName, Distance);
	}
	else
	{
		Blackboard->SetValueAsBool(AICharacterBTCanSeeTargetName, false);
	}
}

bool UBTService_LookForTarget::CanSeeTarget(AAIController* AIController) const
{
	FVector ViewPointLocation;
	FRotator ViewPointRotation;
	AIController->GetActorEyesViewPoint(ViewPointLocation, ViewPointRotation);

	FHitResult HitResult;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(AIController->GetPawn());

	FVector EndLocation = ViewPointLocation + ViewPointRotation.Vector() * TraceRange;

	if (GetWorld()->LineTraceSingleByChannel(HitResult, ViewPointLocation, EndLocation, ECC_Pawn, CollisionQueryParams))
	{
		AActor* SeenActor = HitResult.GetActor();
		return IsValid(SeenActor) && SeenActor->IsA<APlayerCharacter>();
	}

	return false;
}
