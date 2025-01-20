// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Controllers/TurretAIController.h"

#include "AI/Characters/Turret.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Sight.h"

void ATurretAIController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	if (CachedTurret.IsValid())
	{
		CachedTurret->OnTakeAnyDamage.RemoveDynamic(this, &ATurretAIController::OnPawnDamageTaken);
	}

	if (IsValid(InPawn))
	{
		checkf(InPawn->IsA<ATurret>(), TEXT("ATurretAIController::SetPawn TurretAIController can posses only ATurret objects"));
		CachedTurret = StaticCast<ATurret*>(InPawn);

		CachedTurret->OnTakeAnyDamage.AddDynamic(this, &ATurretAIController::OnPawnDamageTaken);
	}
	else
	{
		CachedTurret = nullptr;
	}
}

void ATurretAIController::ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	//Super::ActorsPerceptionUpdated(UpdatedActors);

	if (!CachedTurret.IsValid())
	{
		return;
	}

	const FAISenseAffiliationFilter AISenseAffiliationFilter = CachedTurret->GetAISenseAffiliationFilter();
	AActor* NewTarget = GetClosestSensedActor(UAISense_Damage::StaticClass(), AISenseAffiliationFilter);
	if (!IsValid(NewTarget))
	{
		NewTarget = GetClosestSensedActor(UAISense_Sight::StaticClass(), AISenseAffiliationFilter);
	}

	CachedTurret->SetCurrentTarget(NewTarget);
}
