// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_LookForTarget.generated.h"

/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API UBTService_LookForTarget : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_LookForTarget();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI parameters")
	float TraceRange = 5000.f;

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	bool CanSeeTarget(AAIController* AIController) const;
};
