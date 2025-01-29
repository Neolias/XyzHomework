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
	ExplosionComponent->Explode();
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
	Explode();
}

void AExplosiveProjectile::OnCollisionComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bShouldDetonateOnCollision)
	{
		OnDetonationTimerElapsed();
	}
}
