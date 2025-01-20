// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Controllers/AICharacterController.h"

#include "XyzHomeworkTypes.h"
#include "AI/Characters/AICharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CharacterComponents/AIPatrollingComponent.h"
#include "Perception/AISense.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Sight.h"

void AAICharacterController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	if (CachedAICharacter.IsValid())
	{
		CachedAICharacter->OnTakeAnyDamage.RemoveDynamic(this, &AAICharacterController::OnPawnDamageTaken);
	}

	if (IsValid(InPawn))
	{
		checkf(InPawn->IsA<AAICharacter>(), TEXT("AAICharacterController::SetPawn TurretAIController can posses only AAICharacter objects"));
		CachedAICharacter = StaticCast<AAICharacter*>(InPawn);
		RunBehaviorTree(CachedAICharacter->GetBehaviorTree());

		CachedAICharacter->OnTakeAnyDamage.AddDynamic(this, &AAICharacterController::OnPawnDamageTaken);
	}
	else
	{
		CachedAICharacter = nullptr;
	}
}

void AAICharacterController::BeginPlay()
{
	Super::BeginPlay();

	TryMoveToNextWayPoint();
}

void AAICharacterController::ResetCurrentTarget()
{
	Blackboard->SetValueAsObject(AICharacterBTCurrentTargetName, nullptr);
	ClearFocus(EAIFocusPriority::Gameplay);
}

void AAICharacterController::ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	//Super::ActorsPerceptionUpdated(UpdatedActors);

	if ((CachedAICharacter.IsValid() && !CachedAICharacter->ShouldFollowEnemies()) || !IsValid(Blackboard))
	{
		return;
	}

	const FAISenseAffiliationFilter AISenseAffiliationFilter = CachedAICharacter->GetAISenseAffiliationFilter();
	AActor* ClosestSensedActor = GetClosestSensedActor(UAISense_Damage::StaticClass(), AISenseAffiliationFilter);
	if (!IsValid(ClosestSensedActor))
	{
		ClosestSensedActor = GetClosestSensedActor(UAISense_Sight::StaticClass(), AISenseAffiliationFilter);
	}

	if (IsValid(ClosestSensedActor))
	{
		Blackboard->SetValueAsObject(AICharacterBTCurrentTargetName, ClosestSensedActor);
		SetFocus(ClosestSensedActor);
		bIsPatrolling = false;
	}
	else
	{
		Blackboard->SetValueAsObject(AICharacterBTCurrentTargetName, nullptr);
		ClearFocus(EAIFocusPriority::Gameplay);
		TryMoveToNextWayPoint();
	}
}

void AAICharacterController::OnMoveCompleted(const FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);
	if (!Result.IsSuccess())
	{
		bIsPatrolling = false;
	}

	TryMoveToNextWayPoint();
}

void AAICharacterController::TryMoveToNextWayPoint()
{
	if (!CachedAICharacter.IsValid() || !IsValid(Blackboard))
	{
		return;
	}

	UAIPatrollingComponent* PatrollingComponent = CachedAICharacter->GetPatrollingComponent();

	if (bIsPatrolling)
	{
		FVector NextWayPoint;
		if (PatrollingComponent->GetNextWayPoint(NextWayPoint))
		{
			Blackboard->SetValueAsVector(AICharacterBTNextLocationName, NextWayPoint);
		}
	}
	else
	{
		FVector ClosestWayPoint;
		if (PatrollingComponent->GetClosestWayPoint(ClosestWayPoint, CachedAICharacter->GetActorLocation()))
		{
			Blackboard->SetValueAsVector(AICharacterBTNextLocationName, ClosestWayPoint);
			bIsPatrolling = true;
		}
	}
}
