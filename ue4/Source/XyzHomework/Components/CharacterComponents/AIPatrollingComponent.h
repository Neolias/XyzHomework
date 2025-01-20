// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Components/ActorComponent.h"
#include "AIPatrollingComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XYZHOMEWORK_API UAIPatrollingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAIPatrollingComponent();
	bool GetClosestWayPoint(OUT FVector& ClosestWayPoint, FVector CurrentLocation);
	bool GetNextWayPoint(OUT FVector& NextWayPoint);

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Patrol Parameters")
	class APatrollingPath* PatrollingPath;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Patrol Parameters")
	EPatrolMode PatrolMode = EPatrolMode::Circle;

private:
	int32 CurrentWayPointIndex = -1;
	bool bIsGoingInOppositeDirection = false;
};
