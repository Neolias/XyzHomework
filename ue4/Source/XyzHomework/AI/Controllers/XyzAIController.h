// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "XyzAIController.generated.h"

/**
 *
 */
UCLASS()
class XYZHOMEWORK_API AXyzAIController : public AAIController
{
	GENERATED_BODY()

public:
	AXyzAIController();

	virtual AActor* GetClosestSensedActor(const TSubclassOf<class UAISense> SenseClass, const FAISenseAffiliationFilter& AffiliationFilter) const;

protected:
	UFUNCTION()
	virtual void OnPawnDamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
};
