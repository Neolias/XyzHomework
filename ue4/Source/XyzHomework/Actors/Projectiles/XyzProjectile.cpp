// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectiles/XyzProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AXyzProjectile::AXyzProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetCollisionProfileName("BlockAllDynamic");
	CollisionComponent->InitSphereRadius(5.f);
	RootComponent = CollisionComponent;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(RootComponent);
	StaticMeshComponent->SetIsReplicated(true);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->SetUpdatedComponent(CollisionComponent);
	ProjectileMovementComponent->MaxSpeed = MaxSpeed;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = true;
	ProjectileMovementComponent->Bounciness = Bounciness;
	ProjectileMovementComponent->SetAutoActivate(false);

	SetReplicates(true);
	SetReplicateMovement(true);
	SetProjectileActive(false);
}

void AXyzProjectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionComponent->OnComponentHit.AddDynamic(this, &AXyzProjectile::OnCollisionComponentHit);
}

void AXyzProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AXyzProjectile::SetProjectileActive(const bool bIsActive)
{
	ProjectileMovementComponent->SetActive(bIsActive);
	SetActorTickEnabled(bIsActive);
	SetActorEnableCollision(bIsActive);
	StaticMeshComponent->SetVisibility(bIsActive, true);
}

void AXyzProjectile::Launch(const FVector Direction, const FVector ProjectileResetLocation/* = FVector::ZeroVector*/)
{
	CollisionComponent->IgnoreActorWhenMoving(GetOwningPawn(), true);
	ProjectileMovementComponent->MaxSpeed = MaxSpeed;
	ProjectileMovementComponent->Velocity = LaunchSpeed * Direction;
	ProjectileMovementComponent->Bounciness = Bounciness;
	ResetLocation = ProjectileResetLocation;

	OnProjectileLaunched();
}

APawn* AXyzProjectile::GetOwningPawn() const
{
	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (!IsValid(PawnOwner))
	{
		PawnOwner = Cast<APawn>(GetOwner()->GetOwner());
	}
	return PawnOwner;
}

void AXyzProjectile::OnCollisionComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OnCollisionComponentHitEvent.IsBound())
	{
		OnCollisionComponentHitEvent.Broadcast(this, ProjectileMovementComponent->Velocity.GetSafeNormal(), Hit, ResetLocation);
	}
}
