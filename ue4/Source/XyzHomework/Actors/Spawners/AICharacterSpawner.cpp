// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Spawners/AICharacterSpawner.h"

// Sets default values
AAICharacterSpawner::AAICharacterSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AAICharacterSpawner::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAICharacterSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

