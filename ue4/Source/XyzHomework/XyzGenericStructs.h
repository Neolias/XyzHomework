#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Actors/Projectiles/XyzProjectile.h"
#include "DamageTypes/Weapons/BulletDamageType.h"
#include "XyzGenericStructs.generated.h"

class UNiagaraSystem;
class UCurveVector;

USTRUCT(BlueprintType)
struct FMantlingMovementParameters
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mantling Parameters")
	FVector InitialLocation = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mantling Parameters")
	FRotator InitialRotation = FRotator::ZeroRotator;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mantling Parameters")
	FVector TargetLocation = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mantling Parameters")
	FRotator TargetRotation = FRotator::ZeroRotator;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mantling Parameters")
	UPrimitiveComponent* TargetActor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mantling Parameters")
	FVector InitialAnimationLocation = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mantling Parameters", meta = (ClampMin = 0.f, UIMin = 0.f))
	float Duration = 1.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mantling Parameters", meta = (ClampMin = 0.f, UIMin = 0.f))
	float StartTime = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mantling Parameters")
	UCurveVector* MantlingCurve;
};

USTRUCT(BlueprintType)
struct FLedgeDescription
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ledge Description")
	FVector TargetLocation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ledge Description")
	FRotator TargetRotation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ledge Description")
	FVector LedgeNormal;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ledge Description")
	UPrimitiveComponent* LedgeActor;
};

USTRUCT(BlueprintType)
struct FMantlingSettings
{
	GENERATED_BODY();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* MantlingMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* FPMantlingMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UCurveVector* MantlingCurve;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AnimationCorrectionXY = 65.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AnimationCorrectionZ = 200.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.f, UIMin = 0.f))
	float MaxHeight = 225.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.f, UIMin = 0.f))
	float MinHeight = 125.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.f, UIMin = 0.f))
	float MaxHeightStartTime = 0.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.f, UIMin = 0.f))
	float MinHeightStartTime = 0.6f;
};

USTRUCT(BlueprintType)
struct FDecalInfo
{
	GENERATED_BODY();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UMaterialInterface* DecalMaterial;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector DecalSize = FVector(5.0f, 5.0f, 5.0f);
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DecalLifeTime = 10.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DecalFadeOutTime = 5.0f;
};

USTRUCT(BlueprintType)
struct FWeaponModeParameters
{
	GENERATED_BODY();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo")
	EWeaponAmmoType AmmoType = EWeaponAmmoType::Pistol;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo")
	EHitRegistrationType HitRegistrationType = EHitRegistrationType::HitScan;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (EditCondition = "HitRegistrationType == EHitRegistrationType::Projectile"))
	TSubclassOf<AXyzProjectile> ProjectileClass = AXyzProjectile::StaticClass();
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = 1, UIMin = 1))
	int32 MagazineSize = 10;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo")
	EWeaponReloadType ReloadType = EWeaponReloadType::ByClip;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shooting")
	EWeaponFireMode FireMode = EWeaponFireMode::Single;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shooting", meta = (ClampMin = 0.f, UIMin = 0.f))
	float FireRate = 600.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooting", meta = (ClampMin = 1, UIMin = 1))
	int32 AmmoPerShot = 1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shooting", meta = (ClampMin = 0.f, UIMin = 0.f))
	float FirstShotDelay = 0.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shooting", meta = (ClampMin = 0.f, UIMin = 0.f))
	float MaxBulletSpreadAngle = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = 0.f, UIMin = 0.f))
	float WeaponRange = 5000.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = 0.f, UIMin = 0.f))
	float WeaponMaxDamage = 10.f;
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
	UAnimMontage* WeaponReloadAnimMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
	UAnimMontage* CharacterReloadAnimMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
	UAnimMontage* IronsightsWeaponReloadAnimMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
	UAnimMontage* IronsightsCharacterReloadAnimMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
	FName ReloadLoopStartSectionName = "ReloadLoopStart";
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
	FName ReloadEndSectionName = "ReloadEnd";
};

USTRUCT(BlueprintType)
struct FMeleeAttackDescription
{
	GENERATED_BODY();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.f, UIMin = 0.f))
	float DamageAmount = 50.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class UDamageType> DamageTypeClass = UDamageType::StaticClass();
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* AnimMontage;
};

USTRUCT(BlueprintType)
struct FProjectilePool
{
	GENERATED_BODY();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AXyzProjectile> ProjectileClass = AXyzProjectile::StaticClass();
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 1, UIMin = 1))
	int32 PoolSize = 10;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector PoolWorldLocation = FVector(0.f, 0.f, -1000.f);

	void InstantiatePool(UWorld* World, AActor* PoolOwner_In)
	{
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

	AXyzProjectile* GetNextAvailableProjectile()
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

	TArray<AXyzProjectile*> GetProjectiles()
	{
		return Projectiles;
	}

private:
	UPROPERTY()
	AActor* PoolOwner;
	UPROPERTY()
	TArray<AXyzProjectile*> Projectiles;
	UPROPERTY()
	int32 CurrentProjectileIndex = -1;

};
