// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Equipment/Weapons/MeleeWeaponItem.h"

#include "XyzGenericStructs.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/WeaponComponents/MeleeHitRegistrationComponent.h"

AMeleeWeaponItem::AMeleeWeaponItem()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("WeaponMeshComponent");
	MeshComponent->SetupAttachment(RootComponent);

	EquipmentItemType = EEquipmentItemType::Knife;
}

void AMeleeWeaponItem::BeginPlay()
{
	Super::BeginPlay();

	GetComponents<UMeleeHitRegistrationComponent>(HitRegistrationComponents);
	for (UMeleeHitRegistrationComponent* HitRegistrationComponent : HitRegistrationComponents)
	{
		HitRegistrationComponent->OnHitRegisteredEvent.AddDynamic(this, &AMeleeWeaponItem::ProcessHit);
	}
}

void AMeleeWeaponItem::StartAttack(const EMeleeAttackType AttackType)
{
	if (!CachedBaseCharacterOwner.IsValid())
	{
		return;
	}

	CurrentAttackDescription = Attacks.Find(AttackType);
	if (!CurrentAttackDescription)
	{
		return;
	}

	if (OnAttackActivatedEvent.IsBound())
	{
		OnAttackActivatedEvent.Broadcast(true);
	}

	UAnimMontage* AttackAnimMontage = CurrentAttackDescription->AnimMontage;
	if (IsValid(AttackAnimMontage))
	{
		HitActors.Empty();
		const float Duration = CachedBaseCharacterOwner->PlayAnimMontage(AttackAnimMontage) / AttackAnimMontage->RateScale;
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AMeleeWeaponItem::EndAttack, Duration);
	}
	else
	{
		EndAttack();
	}
}

void AMeleeWeaponItem::EndAttack()
{
	if (OnAttackActivatedEvent.IsBound())
	{
		OnAttackActivatedEvent.Broadcast(false);
	}
}

void AMeleeWeaponItem::EnableHitRegistration(const bool bIsHitRegistrationEnabled)
{
	HitActors.Empty();

	for (UMeleeHitRegistrationComponent* HitRegistrationComponent : HitRegistrationComponents)
	{
		HitRegistrationComponent->EnableHitRegistration(bIsHitRegistrationEnabled);
	}
}

void AMeleeWeaponItem::ProcessHit(const FVector MovementDirection, const FHitResult& HitResult)
{
	if (!CurrentAttackDescription)
	{
		return;
	}

	AActor* HitActor = HitResult.GetActor();
	if (IsValid(HitActor) && !HitActors.Contains(HitActor))
	{
		AController* Controller = CachedBaseCharacterOwner->GetController();
		if (IsValid(Controller))
		{
			FPointDamageEvent DamageEvent;
			DamageEvent.DamageTypeClass = CurrentAttackDescription->DamageTypeClass;
			DamageEvent.HitInfo = HitResult;
			DamageEvent.ShotDirection = MovementDirection;
			HitActor->TakeDamage(CurrentAttackDescription->DamageAmount, DamageEvent, Controller, CachedBaseCharacterOwner.Get());

			HitActors.Add(HitActor);
		}
	}
}
