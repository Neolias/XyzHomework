// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "MeleeHitRegistrationComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHitRegistered, const FVector, MovementDirection, const FHitResult&, Hit);
/**
 *
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class XYZHOMEWORK_API UMeleeHitRegistrationComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	FOnHitRegistered OnHitRegisteredEvent;

	UMeleeHitRegistrationComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	bool IsHitRegistrationEnabled() const { return bIsHitRegistrationEnabled; }
	void EnableHitRegistration(const bool bIsHitRegistrationEnabled_In);

private:
	bool bIsHitRegistrationEnabled = false;
	FVector PreviousComponentLocation = FVector::ZeroVector;

	void ProcessHitRegistration();
};
