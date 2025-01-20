// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericStructs.h"
#include "Actors/Equipment/EquipmentItem.h"
#include "MeleeWeaponItem.generated.h"

class UMeleeHitRegistrationComponent;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAttackActivated, bool);
/**
 *
 */
UCLASS()
class XYZHOMEWORK_API AMeleeWeaponItem : public AEquipmentItem
{
	GENERATED_BODY()

public:
	FOnAttackActivated OnAttackActivatedEvent;

	AMeleeWeaponItem();
	virtual void BeginPlay() override;
	void StartAttack(EMeleeAttackType AttackType);
	void EnableHitRegistration(bool bIsHitRegistrationEnabled);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Parameters | Mesh")
	UStaticMeshComponent* MeshComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Parameters | Attacks")
	TMap<EMeleeAttackType, FMeleeAttackDescription> Attacks;

	UFUNCTION()
	void ProcessHit(FVector MovementDirection, const FHitResult& HitResult);

private:
	const FMeleeAttackDescription* CurrentAttackDescription;
	TArray<UMeleeHitRegistrationComponent*> HitRegistrationComponents;
	TArray<AActor*> HitActors;
	FTimerHandle AttackTimer;

	void EndAttack();
};
