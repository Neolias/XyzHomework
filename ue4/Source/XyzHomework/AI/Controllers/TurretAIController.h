// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzAIController.h"
#include "TurretAIController.generated.h"

/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API ATurretAIController : public AXyzAIController
{
	GENERATED_BODY()

protected:
	virtual void SetPawn(APawn* InPawn) override;
	virtual void ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors) override;	

private:
	TWeakObjectPtr<class ATurret> CachedTurret;
};
