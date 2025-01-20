// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AIComponents/TurretMuzzleComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "XyzHomeworkTypes.h"
#include "DrawDebugHelpers.h"
#include "NiagaraComponent.h"
#include "Components/DecalComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/DebugSubsystem.h"


UTurretMuzzleComponent::UTurretMuzzleComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

APawn* UTurretMuzzleComponent::GetOwningPawn() const
{
	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (!IsValid(PawnOwner))
	{
		PawnOwner = Cast<APawn>(GetOwner()->GetOwner());
	}
	return PawnOwner;
}

AController* UTurretMuzzleComponent::GetController() const
{
	const APawn* PawnOwner = GetOwningPawn();
	return IsValid(PawnOwner) ? PawnOwner->GetController() : nullptr;
}

void UTurretMuzzleComponent::StartFire()
{
	GetWorld()->GetTimerManager().SetTimer(FireTimer, [=] {Shoot(GetController()); }, 60.f / FireRate, true, FirstShotDelay);
}

void UTurretMuzzleComponent::StopFire()
{
	GetWorld()->GetTimerManager().ClearTimer(FireTimer);
}

void UTurretMuzzleComponent::Shoot(AController* Controller) const
{
	FHitResult HitResult;
	const FVector MuzzleLocation = GetComponentLocation();
	const FRotator MuzzleRotation = GetComponentRotation();
	const FVector ShotDirection = GetShotDirection(MuzzleRotation);
	const FVector EndLocation = MuzzleLocation + ShotDirection * WeaponRange;
	FVector TraceEnd = EndLocation;

	if (IsValid(MuzzleFlashFX))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), MuzzleFlashFX, MuzzleLocation, MuzzleRotation);
	}

	if (GetWorld()->LineTraceSingleByChannel(HitResult, MuzzleLocation, EndLocation, ECC_Bullet))
	{
#if (UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT) && ENABLE_DRAW_DEBUG
		const UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
		if (DebugSubsystem->IsCategoryEnabled(DebugCategoryRangedWeapon))
		{
			DrawDebugLine(GetWorld(), MuzzleLocation, HitResult.ImpactPoint, FColor::Red, false, 2.f, 0, 1.f);
		}
#endif

		ProcessHit(HitResult.ImpactPoint - MuzzleLocation, HitResult);
		TraceEnd = HitResult.ImpactPoint;
	}

	if (IsValid(BulletTraceFX))
	{
		UNiagaraComponent* BulletTraceFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), BulletTraceFX, MuzzleLocation, MuzzleRotation);
		BulletTraceFXComponent->SetVectorParameter(TraceEndParamName, TraceEnd);
	}
}

FVector UTurretMuzzleComponent::GetShotDirection(FRotator MuzzleRotation) const
{
	const float SpreadAngle = FMath::FRandRange(0.f, MaxBulletSpreadAngle);
	const float SpreadRoll = FMath::FRandRange(0.f, 360.f);
	MuzzleRotation.Yaw += SpreadAngle * FMath::Cos(SpreadRoll);
	MuzzleRotation.Pitch += SpreadAngle * FMath::Sin(SpreadRoll);
	return MuzzleRotation.Vector();
}

void UTurretMuzzleComponent::ProcessHit(FVector MovementDirection, const FHitResult& HitResult) const
{
#if (UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT) && ENABLE_DRAW_DEBUG
	const UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	if (DebugSubsystem->IsCategoryEnabled(DebugCategoryRangedWeapon))
	{
		DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 10.f, 12, FColor::Red, false, 2.f);
	}
#endif


	FDecalInfo DecalInfo = DefaultDecalInfo;
	if (IsValid(DecalInfo.DecalMaterial))
	{
		UDecalComponent* DecalComponent = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), DecalInfo.DecalMaterial, DecalInfo.DecalSize, HitResult.ImpactPoint, HitResult.ImpactNormal.ToOrientationRotator());
		if (IsValid(DecalComponent))
		{
			DecalComponent->SetFadeScreenSize(0.0001f);
			DecalComponent->SetFadeOut(DecalInfo.DecalLifeTime, DecalInfo.DecalFadeOutTime);
		}
	}

	APawn* PawnOwner = GetOwningPawn();
	if (!IsValid(PawnOwner) || PawnOwner->GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	FPointDamageEvent DamageEvent;
	DamageEvent.HitInfo = HitResult;
	DamageEvent.ShotDirection = MovementDirection;
	DamageEvent.DamageTypeClass = DamageTypeClass;
	AActor* DamagedActor = HitResult.GetActor();
	if (IsValid(DamagedActor))
	{
		float DamageFallOff = 0.f;
		if (IsValid(WeaponDamageFallOff))
		{
			const float Distance = FVector::Dist(GetComponentLocation(), HitResult.ImpactPoint);
			DamageFallOff = WeaponDamageFallOff->GetFloatValue(Distance / WeaponRange);
		}

		DamagedActor->TakeDamage(WeaponMaxDamage * DamageFallOff, DamageEvent, GetController(), PawnOwner);
	}
}
