// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AICharacterSpawner.generated.h"


UCLASS()
class XYZHOMEWORK_API AAICharacterSpawner : public AActor
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void SpawnAI();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	virtual void BeginPlay() override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "AISpawner"))
	bool bSpawnAtBeginPlay = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "AISpawner"))
	bool bSpawnOnce = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "AISpawner"))
	TSubclassOf<class AAICharacter> AICharacterClass;
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, meta = (Category = "AISpawner", ToolTip = "IInteractable object"))
	AActor* SpawnTriggerActor;

private:
	void UpdateSpawnTrigger();
	void UnsubscribeFromTrigger();

	TScriptInterface<class IInteractable> SpawnTrigger;
	bool bCanSpawn = true;
	FDelegateHandle OnSpawnDelegate;
};
