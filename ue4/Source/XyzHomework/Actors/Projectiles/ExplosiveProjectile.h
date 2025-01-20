// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Projectiles/XyzProjectile.h"
#include "ExplosiveProjectile.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProjectileExplosionEvent, AExplosiveProjectile*, ExplosiveProjectile, const FVector, ResetLocation);

UCLASS()
class XYZHOMEWORK_API AExplosiveProjectile : public AXyzProjectile
{
	GENERATED_BODY()

public:
	FOnProjectileExplosionEvent OnProjectileExplosionEvent;

	AExplosiveProjectile();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile | Components")
	class UExplosionComponent* ExplosionComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile | Explosion")
	bool bShouldDetonateOnCollision = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile | Explosion", meta = (ClampMin = 0.f, UIMin = 0.f))
	float DetonationTime = 2.f;

	FTimerHandle DetonationTimer;

	void Explode();
	virtual void OnProjectileLaunched() override;
	void OnDetonationTimerElapsed();
	AController* GetController() const;
	virtual void OnCollisionComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnDetonation();
};
