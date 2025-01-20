// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/WeaponComponents/MeleeHitRegistrationComponent.h"

#include "DrawDebugHelpers.h"
#include "XyzHomeworkTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/DebugSubsystem.h"

UMeleeHitRegistrationComponent::UMeleeHitRegistrationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UMeleeHitRegistrationComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsHitRegistrationEnabled)
	{		
		ProcessHitRegistration();
	}
}

void UMeleeHitRegistrationComponent::EnableHitRegistration(const bool bIsHitRegistrationEnabled_In)
{
	PreviousComponentLocation = bIsHitRegistrationEnabled_In ? GetComponentLocation() : FVector::ZeroVector;
	bIsHitRegistrationEnabled = bIsHitRegistrationEnabled_In;
}

void UMeleeHitRegistrationComponent::ProcessHitRegistration()
{
	const FVector CurrentComponentLocation = GetComponentLocation();
	const FVector TraceVector = CurrentComponentLocation - PreviousComponentLocation;
	FHitResult HitResult;
	const UWorld* World = GetWorld();
	const bool bHasHit = World->SweepSingleByChannel(HitResult, PreviousComponentLocation, CurrentComponentLocation,
		FQuat::Identity, ECC_Melee, FCollisionShape::MakeSphere(SphereRadius),
		FCollisionQueryParams::DefaultQueryParam, FCollisionResponseParams::DefaultResponseParam);

#if (UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT) && ENABLE_DRAW_DEBUG
	const UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	if (DebugSubsystem->IsCategoryEnabled(DebugCategoryMeleeWeapon))
	{
		const FVector CapsuleCenter = (PreviousComponentLocation + CurrentComponentLocation) / 2;
		const FQuat CapsuleRotation = FRotationMatrix::MakeFromZ(TraceVector).ToQuat();

		DrawDebugCapsule(World, CapsuleCenter, TraceVector.Size() / 2, SphereRadius, CapsuleRotation, FColor::Magenta, false, 3.f);
	}
#endif

	if (bHasHit && OnHitRegisteredEvent.IsBound())
	{
		OnHitRegisteredEvent.Broadcast(TraceVector, HitResult);
	}

	PreviousComponentLocation = CurrentComponentLocation;
}
