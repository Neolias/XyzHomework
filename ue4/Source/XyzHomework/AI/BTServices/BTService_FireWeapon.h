// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "Characters/XyzBaseCharacter.h"
#include "BTService_FireWeapon.generated.h"

/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API UBTService_FireWeapon : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_FireWeapon();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

};
