// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "XyzProjectile.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnCollisionComponentHit, AXyzProjectile*, Projectile, const FVector, MovementDirection, const FHitResult&, Hit, const FVector, ResetLocation);

UCLASS()
class XYZHOMEWORK_API AXyzProjectile : public AActor
{
	GENERATED_BODY()
	
public:
	FOnCollisionComponentHit OnCollisionComponentHitEvent;

	AXyzProjectile();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	void SetProjectileActive(bool bIsActive);
	virtual void Launch(FVector Direction, FVector ProjectileResetLocation = FVector::ZeroVector);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile | Components")
	class USphereComponent* CollisionComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile | Components")
	UStaticMeshComponent* StaticMeshComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile | Components")
	class UProjectileMovementComponent* ProjectileMovementComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile | Movement", meta = (ClampMin = 0.f, UIMin = 0.f))
	float LaunchSpeed = 1500.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile | Movement", meta = (ClampMin = 0.f, UIMin = 0.f))
	float MaxSpeed = 3000.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile | Movement", meta = (ClampMin = 0.f, UIMin = 0.f))
	float Bounciness = 0.1f;

	FVector ResetLocation = FVector::ZeroVector;

	APawn* GetOwningPawn() const;
	virtual void OnProjectileLaunched() {}
	UFUNCTION()
	virtual void OnCollisionComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
