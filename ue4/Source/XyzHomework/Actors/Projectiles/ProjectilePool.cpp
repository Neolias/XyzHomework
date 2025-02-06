// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectilePool.h"

#include "XyzProjectile.h"

void FProjectilePool::InstantiatePool(UWorld* World, AActor* PoolOwner_In)
{
	if (!IsValid(ProjectileClass))
	{
		return;
	}

	PoolOwner = PoolOwner_In;
	Projectiles.Reserve(PoolSize);
	for (int i = 0; i < PoolSize; ++i)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = PoolOwner_In;
		AXyzProjectile* Projectile = World->SpawnActor<AXyzProjectile>(ProjectileClass, PoolWorldLocation, FRotator::ZeroRotator, SpawnParameters);
		Projectiles.Add(Projectile);
	}
}

AXyzProjectile* FProjectilePool::GetNextAvailableProjectile()
{
	const int32 Size = Projectiles.Num();

	if (Size == 0)
	{
		return nullptr;
	}

	if (PoolOwner->GetLocalRole() == ROLE_Authority)
	{
		CurrentProjectileIndex++;
	}

	if (CurrentProjectileIndex == -1)
	{
		return nullptr;
	}

	if (CurrentProjectileIndex >= Size)
	{
		CurrentProjectileIndex = 0;
	}

	return Projectiles[CurrentProjectileIndex];
}
