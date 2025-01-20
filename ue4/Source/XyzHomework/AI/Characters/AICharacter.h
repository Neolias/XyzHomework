// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/XyzBaseCharacter.h"
#include "Perception/AIPerceptionTypes.h"
#include "AICharacter.generated.h"

class UAIPatrollingComponent;
class UBehaviorTree;
/**
 * 
 */
UCLASS(Blueprintable)
class XYZHOMEWORK_API AAICharacter : public AXyzBaseCharacter
{
	GENERATED_BODY()

public:
	AAICharacter(const OUT FObjectInitializer& ObjectInitializer);
	UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }
	FAISenseAffiliationFilter GetAISenseAffiliationFilter() const { return AISenseAffiliationFilter; }
	UAIPatrollingComponent* GetPatrollingComponent() const { return PatrollingComponent; }
	bool ShouldFollowEnemies() const { return bShouldFollowEnemies; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Character | Behavior Tree")
	UBehaviorTree* BehaviorTree;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Character | Perception")
	FAISenseAffiliationFilter AISenseAffiliationFilter;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Character | Navigaton")
	UAIPatrollingComponent* PatrollingComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Character | Navigaton")
	bool bShouldFollowEnemies = true;
};
