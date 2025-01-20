// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Equipment/Throwables/ThrowableItem.h"

#include "Actors/Projectiles/ExplosiveProjectile.h"
#include "Actors/Projectiles/XyzProjectile.h"
#include "Characters/XyzBaseCharacter.h"

AThrowableItem::AThrowableItem()
{
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	StaticMesh->SetupAttachment(RootComponent);
}

void AThrowableItem::Throw(AXyzProjectile* ThrowableProjectile, const FVector ResetLocation)
{
	if (!CachedBaseCharacterOwner.IsValid() || !IsValid(ThrowableProjectile))
	{
		return;
	}

	CurrentProjectile = ThrowableProjectile;
	ProjectileResetLocation = ResetLocation;

	if (IsValid(ThrowItemAnimMontage))
	{
		const float Duration = CachedBaseCharacterOwner->PlayAnimMontage(ThrowItemAnimMontage);
		GetWorld()->GetTimerManager().SetTimer(ThrowAnimationTimer, this, &AThrowableItem::OnThrowAnimationFinished, Duration, false);
		bIsThrowing = true;
	}
}

void AThrowableItem::LaunchProjectile() const
{
	if (!CachedBaseCharacterOwner.IsValid() || !CurrentProjectile.IsValid())
	{
		return;
	}

	const AController* Controller = CachedBaseCharacterOwner->GetInstigatorController();
	if (!IsValid(Controller))
	{
		return;
	}

	const USkeletalMeshComponent* CharacterMesh = CachedBaseCharacterOwner->GetMesh();
	if (!IsValid(CharacterMesh))
	{
		return;
	}

	FVector POVLocation;
	FRotator POVRotation;
	Controller->GetPlayerViewPoint(POVLocation, POVRotation);
	const FVector ThrowableSocketLocation = CharacterMesh->GetSocketLocation(ThrowableSocketName);
	const FVector OffsetFromPOV = ThrowableSocketLocation - POVLocation;
	const FVector POVForwardVector = POVRotation.Vector();
	const float ForwardOffsetFromPOV = FVector::DotProduct(OffsetFromPOV, POVForwardVector);
	const FVector StartLocation = POVLocation + POVForwardVector * ForwardOffsetFromPOV;
	const FRotator LaunchRotation = POVRotation + FRotator(LaunchPitchAngle, LaunchYawAngle, 0.f);
	const FVector LaunchDirection = LaunchRotation.Vector();

	CurrentProjectile->SetActorLocation(StartLocation);
	CurrentProjectile->SetActorRotation(LaunchDirection.ToOrientationRotator());
	CurrentProjectile->SetProjectileActive(true);

	AExplosiveProjectile* ExplosiveProjectile = Cast<AExplosiveProjectile>(CurrentProjectile);
	if (IsValid(ExplosiveProjectile))
	{
		ExplosiveProjectile->OnProjectileExplosionEvent.AddDynamic(this, &AThrowableItem::OnProjectileExplosion);
	}
	else
	{
		CurrentProjectile->OnCollisionComponentHitEvent.AddDynamic(this, &AThrowableItem::ProcessThrowableProjectileHit);
	}

	CurrentProjectile->Launch(LaunchDirection, ProjectileResetLocation);

	OnThrowEnd();
}

void AThrowableItem::OnThrowEnd() const
{
	if (OnThrowEndEvent.IsBound())
	{
		OnThrowEndEvent.Broadcast();
	}
}

void AThrowableItem::OnThrowAnimationFinished()
{
	if (OnThrowAnimationFinishedEvent.IsBound())
	{
		OnThrowAnimationFinishedEvent.Broadcast();
	}
	bIsThrowing = false;
}

void AThrowableItem::ProcessThrowableProjectileHit(AXyzProjectile* Projectile, const FVector MovementDirection, const FHitResult& HitResult, const FVector ResetLocation)
{
	ResetThrowableProjectile(Projectile, ResetLocation);
}

void AThrowableItem::OnProjectileExplosion(AExplosiveProjectile* ExplosiveProjectile, const FVector ResetLocation)
{
	ExplosiveProjectile->OnProjectileExplosionEvent.RemoveAll(this);
	ResetThrowableProjectile(ExplosiveProjectile, ResetLocation);
}

void AThrowableItem::ResetThrowableProjectile(AXyzProjectile* Projectile, const FVector ResetLocation)
{
	Projectile->SetProjectileActive(false);
	Projectile->SetActorLocation(ResetLocation);
	Projectile->SetActorRotation(FRotator::ZeroRotator);
	Projectile->OnCollisionComponentHitEvent.RemoveAll(this);

	CurrentProjectile = nullptr;
}
