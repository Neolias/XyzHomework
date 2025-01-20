// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericStructs.h"
#include "Components/SceneComponent.h"
#include "TurretMuzzleComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XYZHOMEWORK_API UTurretMuzzleComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	UTurretMuzzleComponent();
	void StartFire();
	void StopFire();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooting", meta = (ClampMin = 0.f, UIMin = 0.f))
	float FireRate = 150.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooting", meta = (ClampMin = 0.f, UIMin = 0.f))
	float FirstShotDelay = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooting", meta = (ClampMin = 0.f, UIMin = 0.f))
	float MaxBulletSpreadAngle = 5.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = 0.f, UIMin = 0.f))
	float WeaponRange = 5000.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = 0.f, UIMin = 0.f))
	float WeaponMaxDamage = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	UCurveFloat* WeaponDamageFallOff;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	TSubclassOf<class UDamageType> DamageTypeClass = UBulletDamageType::StaticClass();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* MuzzleFlashFX;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* BulletTraceFX;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	FName TraceEndParamName = "TraceEnd";
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Decals")
	FDecalInfo DefaultDecalInfo;

private:
	FTimerHandle FireTimer;

	APawn* GetOwningPawn() const;
	AController* GetController() const;
	void Shoot(AController* Controller) const;
	FVector GetShotDirection(FRotator MuzzleRotation) const;
	void ProcessHit(FVector MovementDirection, const FHitResult& HitResult) const;		
};
