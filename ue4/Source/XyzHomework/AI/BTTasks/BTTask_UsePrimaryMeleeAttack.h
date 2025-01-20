// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_UsePrimaryMeleeAttack.generated.h"

/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API UBTTask_UsePrimaryMeleeAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_UsePrimaryMeleeAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
