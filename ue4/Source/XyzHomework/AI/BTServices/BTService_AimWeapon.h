// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_AimWeapon.generated.h"

/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API UBTService_AimWeapon : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_AimWeapon();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI parameters")
	float MaxAimingRange = 1500.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI parameters")
	float MinAimingRange = 500.f;

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
};
