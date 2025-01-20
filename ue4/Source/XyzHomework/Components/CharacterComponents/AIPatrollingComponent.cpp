// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CharacterComponents/AIPatrollingComponent.h"

#include "Actors/Navigation/PatrollingPath.h"

UAIPatrollingComponent::UAIPatrollingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UAIPatrollingComponent::GetClosestWayPoint(FVector& ClosestWayPoint, const FVector CurrentLocation)
{
	if (!IsValid(PatrollingPath))
	{
		return false;
	}

	const TArray<FVector>& WayPoints = PatrollingPath->GetWayPoints();
	const int32 WayPointsCount = WayPoints.Num();
	if (WayPointsCount < 1)
	{
		return false;
	}

	const FTransform PathTransform = PatrollingPath->GetActorTransform();
	float MaxDistanceSquared = FLT_MAX;

	for (int i = 0; i < WayPointsCount; ++i)
	{
		const FVector WayPointLocation = PathTransform.TransformPosition(WayPoints[i]);
		const float DistanceSquared = (WayPointLocation - CurrentLocation).SizeSquared();
		if (DistanceSquared < MaxDistanceSquared)
		{
			MaxDistanceSquared = DistanceSquared;
			ClosestWayPoint = WayPointLocation;
		}
	}

	CurrentWayPointIndex = WayPoints.Find(ClosestWayPoint);
	return true;
}

bool UAIPatrollingComponent::GetNextWayPoint(FVector& NextWayPoint)
{
	if (!IsValid(PatrollingPath))
	{
		return false;
	}

	const TArray<FVector>& WayPoints = PatrollingPath->GetWayPoints();
	const int32 WayPointsCount = WayPoints.Num();
	if (WayPointsCount < 2)
	{
		return false;
	}

	switch (PatrolMode)
	{
	default:
	case EPatrolMode::None:
		return false;
	case EPatrolMode::Circle:
		CurrentWayPointIndex++;
		if (CurrentWayPointIndex == WayPointsCount)
		{
			CurrentWayPointIndex = 0;
		}
		break;
	case EPatrolMode::PingPong:
		if (bIsGoingInOppositeDirection)
		{
			CurrentWayPointIndex--;
			if (CurrentWayPointIndex <= 0)
			{
				bIsGoingInOppositeDirection = false;
				CurrentWayPointIndex = 0;
			}
		}
		else
		{
			CurrentWayPointIndex++;
			if (CurrentWayPointIndex == WayPointsCount - 1)
			{
				bIsGoingInOppositeDirection = true;
			}
		}
		break;
	}

	const FTransform PathTransform = PatrollingPath->GetActorTransform();
	NextWayPoint = PathTransform.TransformPosition(WayPoints[CurrentWayPointIndex]);
	return true;
}
