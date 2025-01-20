// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Controllers/XyzAIController.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Damage.h"

AXyzAIController::AXyzAIController()
{
	UAIPerceptionComponent* NewPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SetPerceptionComponent(*NewPerceptionComponent);
}

AActor* AXyzAIController::GetClosestSensedActor(const TSubclassOf<UAISense> SenseClass, const FAISenseAffiliationFilter& AffiliationFilter) const
{
	APawn* CurrentPawn = GetPawn();

	if (!IsValid(CurrentPawn) || !IsValid(PerceptionComponent))
	{
		return nullptr;
	}

	TArray<AActor*> SensedActors;
	PerceptionComponent->GetCurrentlyPerceivedActors(SenseClass, SensedActors);

	AActor* SensedActor = nullptr;
	float MaxDistanceSquared = FLT_MAX;
	const FVector PawnLocation = CurrentPawn->GetActorLocation();

	const IGenericTeamAgentInterface* TeamAgentInterface = Cast<IGenericTeamAgentInterface>(CurrentPawn);

	for (AActor* Actor : SensedActors)
	{
		if (!AffiliationFilter.ShouldDetectAll() && TeamAgentInterface)
		{
			const ETeamAttitude::Type TeamAttitude = TeamAgentInterface->GetTeamAttitudeTowards(*Actor);
			switch (TeamAttitude)
			{
			case ETeamAttitude::Hostile:
				if (!AffiliationFilter.bDetectEnemies)
				{
					continue;
				}
				break;
			case ETeamAttitude::Neutral:
				if (!AffiliationFilter.bDetectNeutrals)
				{
					continue;
				}
				break;
			case ETeamAttitude::Friendly:
				if (!AffiliationFilter.bDetectFriendlies)
				{
					continue;
				}
				break;
			default:
				continue;
			}
		}

		const float DistanceSquared = (PawnLocation - Actor->GetActorLocation()).SizeSquared();
		if (DistanceSquared < MaxDistanceSquared)
		{
			MaxDistanceSquared = DistanceSquared;
			SensedActor = Actor;
		}
	}

	return SensedActor;
}

void AXyzAIController::OnPawnDamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (!IsValid(PerceptionComponent))
	{
		return;
	}

	const FAISenseID SenseID = UAISense::GetSenseID<UAISense_Damage>();
	const UAISenseConfig_Damage* SenseConfig = Cast<UAISenseConfig_Damage>(PerceptionComponent->GetSenseConfig(SenseID));
	if (!IsValid(SenseConfig))
	{
		return;
	}

	const UAISense_Damage* Sense = NewObject<UAISense_Damage>(SenseConfig->GetSenseImplementation());
	if (IsValid(Sense))
	{
		Sense->ReportDamageEvent(GetWorld(), DamagedActor, DamageCauser, Damage, DamageCauser->GetActorLocation(), DamagedActor->GetActorLocation());
	}
}