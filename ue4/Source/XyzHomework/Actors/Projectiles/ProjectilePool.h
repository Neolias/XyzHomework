// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectilePool.generated.h"

class AXyzProjectile;

USTRUCT(BlueprintType)
struct FProjectilePool
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AXyzProjectile> ProjectileClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 1, UIMin = 1))
	int32 PoolSize = 10;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector PoolWorldLocation = FVector(0.f, 0.f, -1000.f);

	void InstantiatePool(UWorld* World, AActor* PoolOwner_In);
	AXyzProjectile* GetNextAvailableProjectile();
	TArray<AXyzProjectile*> GetProjectiles() { return Projectiles; }

private:
	UPROPERTY()
	AActor* PoolOwner = nullptr;
	UPROPERTY()
	TArray<AXyzProjectile*> Projectiles;
	UPROPERTY()
	int32 CurrentProjectileIndex = -1;
};