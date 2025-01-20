// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/WeaponComponents/WeaponMuzzleComponent.h"

#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/DecalComponent.h"

#include "DrawDebugHelpers.h"
#include "XyzHomeworkTypes.h"
#include "Actors/Projectiles/ExplosiveProjectile.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/DebugSubsystem.h"

UWeaponMuzzleComponent::UWeaponMuzzleComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UWeaponMuzzleComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwningPawn();
	if (IsValid(Owner) && Owner->GetLocalRole() == ROLE_Authority)
	{
		InstantiateProjectilePools(Owner);
	}
}

void UWeaponMuzzleComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UWeaponMuzzleComponent, ProjectilePools);
}

AController* UWeaponMuzzleComponent::GetController() const
{
	const APawn* PawnOwner = GetOwningPawn();
	return IsValid(PawnOwner) ? PawnOwner->GetController() : nullptr;
}

void UWeaponMuzzleComponent::InstantiateProjectilePools(AActor* Owner)
{
	for (FProjectilePool& Pool : ProjectilePools)
	{
		Pool.InstantiatePool(GetWorld(), Owner);
	}
}

void UWeaponMuzzleComponent::Shoot(const FWeaponModeParameters* WeaponModeParameters, const FVector ViewPointLocation, const FRotator ViewPointRotation)
{
	if (!WeaponModeParameters)
	{
		return;
	}
	ModeParameters = WeaponModeParameters;

	FHitResult HitResult;
	const FVector MuzzleLocation = GetComponentLocation();
	const FVector EndLocation = ViewPointLocation + ViewPointRotation.Vector() * ModeParameters->WeaponRange;

	const APawn* PawnOwner = GetOwningPawn();
	switch (ModeParameters->HitRegistrationType)
	{
	case EHitRegistrationType::Projectile:
		if (IsValid(PawnOwner) && PawnOwner->GetLocalRole() == ROLE_Authority)
		{
			ShootProjectile(WeaponModeParameters->ProjectileClass, MuzzleLocation, ViewPointLocation, ViewPointRotation, EndLocation);
		}
		break;
	case EHitRegistrationType::HitScan:
	default:
		const FVector TraceEnd = ShootHitScan(ViewPointLocation, ViewPointRotation, HitResult, MuzzleLocation, EndLocation);
		if (IsValid(ModeParameters->BulletTraceFX))
		{
			UNiagaraComponent* BulletTraceFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ModeParameters->BulletTraceFX, GetComponentLocation(), GetComponentRotation());
			if (IsValid(BulletTraceFXComponent))
			{
				BulletTraceFXComponent->SetVectorParameter(ModeParameters->TraceEndParamName, TraceEnd);
			}
		}
		break;
	}
}

APawn* UWeaponMuzzleComponent::GetOwningPawn() const
{
	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (!IsValid(PawnOwner))
	{
		PawnOwner = Cast<APawn>(GetOwner()->GetOwner());
	}
	return PawnOwner;
}

void UWeaponMuzzleComponent::ShootProjectile(const TSubclassOf<AXyzProjectile> ProjectileClass, const FVector MuzzleLocation, const FVector ViewPointLocation,
	const FRotator ViewPointRotation, const FVector EndLocation)
{
	FProjectilePool* ProjectilePool = ProjectilePools.FindByPredicate([ProjectileClass](const FProjectilePool& Pool) { return Pool.ProjectileClass == ProjectileClass; });
	if (!ProjectilePool)
	{
		return;
	}

	AXyzProjectile* Projectile = ProjectilePool->GetNextAvailableProjectile();
	if (IsValid(Projectile))
	{
		const FVector OffsetFromPOV = MuzzleLocation - ViewPointLocation;
		const FVector POVForwardVector = ViewPointRotation.Vector();
		const float ForwardOffsetFromPOV = FVector::DotProduct(OffsetFromPOV, POVForwardVector);
		const FVector StartLocation = ViewPointLocation + POVForwardVector * ForwardOffsetFromPOV;
		const FVector LaunchDirection = (EndLocation - StartLocation).GetSafeNormal();

		Server_OnShootProjectile(Projectile, StartLocation, LaunchDirection, ProjectilePool->PoolWorldLocation);
	}
}

void UWeaponMuzzleComponent::Server_OnShootProjectile_Implementation(AXyzProjectile* Projectile, const FVector StartLocation, const FVector LaunchDirection, const FVector ResetLocation)
{
	Multicast_OnShootProjectile(Projectile, StartLocation, LaunchDirection, ResetLocation);
}

void UWeaponMuzzleComponent::Multicast_OnShootProjectile_Implementation(AXyzProjectile* Projectile, const FVector StartLocation, const FVector LaunchDirection, const FVector ResetLocation)
{
	OnShootProjectile(Projectile, StartLocation, LaunchDirection, ResetLocation);
}

void UWeaponMuzzleComponent::OnShootProjectile(AXyzProjectile* Projectile, const FVector StartLocation, const FVector LaunchDirection, const FVector ResetLocation)
{
	Projectile->SetActorLocation(StartLocation);
	Projectile->SetActorRotation(LaunchDirection.ToOrientationRotator());
	Projectile->SetProjectileActive(true);

	AExplosiveProjectile* ExplosiveProjectile = Cast<AExplosiveProjectile>(Projectile);
	if (IsValid(ExplosiveProjectile))
	{
		ExplosiveProjectile->OnProjectileExplosionEvent.AddDynamic(this, &UWeaponMuzzleComponent::OnProjectileExplosion);
	}
	else
	{
		Projectile->OnCollisionComponentHitEvent.AddDynamic(this, &UWeaponMuzzleComponent::ProcessProjectileHit);
	}

	Projectile->Launch(LaunchDirection, ResetLocation);
}

FVector UWeaponMuzzleComponent::ShootHitScan(const FVector ViewPointLocation, const FRotator ViewPointRotation, FHitResult& HitResult,
	const FVector MuzzleLocation, const FVector EndLocation)
{
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(GetOwningPawn());

	if (GetWorld()->LineTraceSingleByChannel(HitResult, ViewPointLocation, EndLocation, ECC_Bullet, CollisionQueryParams))
	{
#if (UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT) && ENABLE_DRAW_DEBUG
		const UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
		if (DebugSubsystem->IsCategoryEnabled(DebugCategoryRangedWeapon))
		{
			DrawDebugLine(GetWorld(), ViewPointLocation, HitResult.ImpactPoint, FColor::Red, false, 2.f, 0, 1.f);
		}
#endif

		ProcessHit(HitResult.ImpactPoint - MuzzleLocation, HitResult);

		return HitResult.ImpactPoint;
	}
	return EndLocation;
}

void UWeaponMuzzleComponent::ProcessProjectileHit(AXyzProjectile* Projectile, const FVector MovementDirection, const FHitResult& HitResult, const FVector ResetLocation)
{
	ResetProjectile(Projectile, ResetLocation);
	ProcessHit(MovementDirection, HitResult);
}

void UWeaponMuzzleComponent::ProcessHit(const FVector MovementDirection, const FHitResult& HitResult)
{
#if (UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT) && ENABLE_DRAW_DEBUG
	const UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	if (DebugSubsystem->IsCategoryEnabled(DebugCategoryRangedWeapon))
	{
		DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 10.f, 12, FColor::Red, false, 2.f);
	}
#endif

	if (!ModeParameters)
	{
		return;
	}

	FDecalInfo DecalInfo = ModeParameters->DefaultDecalInfo;
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
	DamageEvent.DamageTypeClass = ModeParameters->DamageTypeClass;
	AActor* DamagedActor = HitResult.GetActor();
	if (IsValid(DamagedActor))
	{
		float DamageFallOff = 0.f;
		if (IsValid(ModeParameters->WeaponDamageFallOff))
		{
			const float Distance = FVector::Dist(GetComponentLocation(), HitResult.ImpactPoint);
			DamageFallOff = ModeParameters->WeaponDamageFallOff->GetFloatValue(Distance / ModeParameters->WeaponRange);
		}

		DamagedActor->TakeDamage(ModeParameters->WeaponMaxDamage * DamageFallOff, DamageEvent, GetController(), PawnOwner);
	}
}

void UWeaponMuzzleComponent::OnProjectileExplosion(AExplosiveProjectile* ExplosiveProjectile, const FVector ResetLocation)
{
	ExplosiveProjectile->OnProjectileExplosionEvent.RemoveAll(this);
	ResetProjectile(ExplosiveProjectile, ResetLocation);
}

void UWeaponMuzzleComponent::ResetProjectile(AXyzProjectile* Projectile, const FVector ResetLocation)
{
	Projectile->SetProjectileActive(false);
	Projectile->SetActorLocation(ResetLocation);
	Projectile->SetActorRotation(FRotator::ZeroRotator);

	Projectile->OnCollisionComponentHitEvent.RemoveAll(this);
}
