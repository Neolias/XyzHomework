// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ExplosionComponent.h"

#include "Actors/Projectiles/XyzProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

void UExplosionComponent::BeginPlay()
{
	Super::BeginPlay();
	GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UExplosionComponent::OnDamageTaken);
}

APawn* UExplosionComponent::GetOwningPawn() const
{
	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (!IsValid(PawnOwner))
	{
		PawnOwner = Cast<APawn>(GetOwner()->GetOwner());
	}
	return PawnOwner;
}

void UExplosionComponent::OnDamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType_In, AController* InstigatedBy, AActor* DamageCauser)
{
	Explode();
	if (GetOwner()->GetLocalRole() == ROLE_Authority)
	{
		Multicast_OnExplosion();
	}
	if (GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_OnExplosion();
	}
}

void UExplosionComponent::Server_OnExplosion_Implementation()
{
	Explode();
	Multicast_OnExplosion();
}

void UExplosionComponent::Multicast_OnExplosion_Implementation()
{
	if (GetOwner()->GetLocalRole() == ROLE_SimulatedProxy)
	{
		Explode();
	}
}

void UExplosionComponent::Explode() const
{
	if (!IsValid(DamageType))
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (IsValid(Owner))
	{
		Owner->SetActorEnableCollision(false);
		Owner->SetActorHiddenInGame(true);
	}

	AActor* OwnerActor = Owner;
	APawn* OwningPawn = GetOwningPawn();
	AController* Controller = nullptr;
	if (IsValid(OwningPawn))
	{
		OwnerActor = OwningPawn;
		Controller = OwningPawn->GetController();
	}
	
	if (IsValid(OwnerActor) && OwnerActor->GetLocalRole() == ROLE_Authority)
	{
		TArray<AActor*> IgnoreActors;
		IgnoreActors.Add(Owner);
		const FVector DamageLocation = GetComponentLocation();

		UGameplayStatics::ApplyRadialDamageWithFalloff(
			GetWorld(), BaseDamage, MinimumDamage, DamageLocation,
			InnerRadius, OuterRadius, DamageFalloff, DamageType, IgnoreActors,
			OwnerActor, Controller, ECC_Visibility);
	}

	if (IsValid(ExplosionVFX))
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionVFX, GetComponentLocation());
	}

	if (OnExplosionEvent.IsBound())
	{
		OnExplosionEvent.Broadcast();
	}
}
