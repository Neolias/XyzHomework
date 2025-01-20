// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericStructs.h"
#include "Actors/Projectiles/ExplosiveProjectile.h"
#include "Actors/Projectiles/XyzProjectile.h"
#include "Components/SceneComponent.h"
#include "WeaponMuzzleComponent.generated.h"

class UNiagaraSystem;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class XYZHOMEWORK_API UWeaponMuzzleComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UWeaponMuzzleComponent();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void Shoot(const FWeaponModeParameters* WeaponModeParameters, FVector ViewPointLocation, FRotator ViewPointRotation);

protected:
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Muzzle Component | Projectiles")
	TArray<FProjectilePool> ProjectilePools;

private:
	TWeakObjectPtr<APawn> CachedOwningPawn;
	const FWeaponModeParameters* ModeParameters;

	APawn* GetOwningPawn() const;
	AController* GetController() const;
	void InstantiateProjectilePools(AActor* Owner);
	void ShootProjectile(const TSubclassOf<AXyzProjectile> ProjectileClass, FVector MuzzleLocation, FVector ViewPointLocation, FRotator ViewPointRotation, FVector EndLocation);
	void OnShootProjectile(AXyzProjectile* Projectile, FVector StartLocation, FVector LaunchDirection, FVector ResetLocation);
	UFUNCTION(Server, Reliable)
	void Server_OnShootProjectile(AXyzProjectile* Projectile, FVector StartLocation, FVector LaunchDirection, FVector ResetLocation);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnShootProjectile(AXyzProjectile* Projectile, FVector StartLocation, FVector LaunchDirection, FVector ResetLocation);
	FVector ShootHitScan(FVector ViewPointLocation, FRotator ViewPointRotation, OUT FHitResult& HitResult, FVector MuzzleLocation, FVector EndLocation);
	UFUNCTION()
	void ProcessHit(FVector MovementDirection, const FHitResult& HitResult);
	UFUNCTION()
	void ProcessProjectileHit(AXyzProjectile* Projectile, FVector MovementDirection, const FHitResult& HitResult, const FVector ResetLocation);
	UFUNCTION()
	void OnProjectileExplosion(AExplosiveProjectile* ExplosiveProjectile, const FVector ResetLocation);
	void ResetProjectile(AXyzProjectile* Projectile, const FVector ResetLocation);
};
