// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Spawners/AICharacterSpawner.h"

#include "Actors/Interactive/Interactable.h"
#include "AI/Characters/AICharacter.h"

void AAICharacterSpawner::BeginPlay()
{
	Super::BeginPlay();

	UpdateSpawnTrigger();

	if (bSpawnAtBeginPlay)
	{
		SpawnAI();
		UnsubscribeFromTrigger();
	}
}

void AAICharacterSpawner::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UpdateSpawnTrigger();
}

void AAICharacterSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UnsubscribeFromTrigger();
}

void AAICharacterSpawner::SpawnAI()
{
	if (!bCanSpawn || !IsValid(AICharacterClass))
	{
		return;
	}

	AAICharacter* AICharacter = GetWorld()->SpawnActor<AAICharacter>(AICharacterClass, GetTransform());
	if (!IsValid(AICharacter->GetController()))
	{
		AICharacter->SpawnDefaultController();
	}

	if (bSpawnOnce)
	{
		bCanSpawn = false;
	}
}

void AAICharacterSpawner::UpdateSpawnTrigger()
{
	if (SpawnTrigger == SpawnTriggerActor)
	{
		return;
	}

	SpawnTrigger = SpawnTriggerActor;
	if (!SpawnTrigger.GetInterface())
	{
		SpawnTriggerActor = nullptr;
		SpawnTrigger = nullptr;
	}

	if (SpawnTrigger->HasOnInteractionCallback())
	{
		OnSpawnDelegate = SpawnTrigger->AddOnInteractionDelegate(this, FName("SpawnAI"));
	}
}

void AAICharacterSpawner::UnsubscribeFromTrigger()
{
	if (OnSpawnDelegate.IsValid() && SpawnTrigger.GetInterface())
	{
		SpawnTrigger->RemoveOnInteractionDelegate(OnSpawnDelegate);
	}
}

