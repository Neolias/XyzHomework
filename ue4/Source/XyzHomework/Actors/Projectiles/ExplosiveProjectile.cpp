// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectiles/ExplosiveProjectile.h"

#include "Components/ExplosionComponent.h"

AExplosiveProjectile::AExplosiveProjectile()
{
	ExplosionComponent = CreateDefaultSubobject<UExplosionComponent>(TEXT("ExplosionComponoent"));
	ExplosionComponent->SetupAttachment(RootComponent);
}

void AExplosiveProjectile::Explode()
{
	ExplosionComponent->Explode(GetController());
	if (OnProjectileExplosionEvent.IsBound())
	{
		OnProjectileExplosionEvent.Broadcast(this, ResetLocation);
	}
}

void AExplosiveProjectile::OnProjectileLaunched()
{
	Super::OnProjectileLaunched();

	if (!bShouldDetonateOnCollision)
	{
		GetWorld()->GetTimerManager().SetTimer(DetonationTimer, this, &AExplosiveProjectile::OnDetonationTimerElapsed, DetonationTime, false);
	}
}

void AExplosiveProjectile::OnDetonationTimerElapsed()
{
	Multicast_OnDetonation();
}

void AExplosiveProjectile::Multicast_OnDetonation_Implementation()
{
	Explode();
}

AController* AExplosiveProjectile::GetController() const
{
	const APawn* PawnOwner = Cast<APawn>(GetOwner());
	return IsValid(PawnOwner) ? PawnOwner->GetController() : nullptr;
}

void AExplosiveProjectile::OnCollisionComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bShouldDetonateOnCollision)
	{
		OnDetonationTimerElapsed();
	}
}
