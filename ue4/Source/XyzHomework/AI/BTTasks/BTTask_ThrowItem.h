// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ThrowItem.generated.h"

/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API UBTTask_ThrowItem : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ThrowItem();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
