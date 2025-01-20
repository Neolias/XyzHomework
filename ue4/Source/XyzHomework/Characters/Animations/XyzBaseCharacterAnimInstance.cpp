// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Animations/XyzBaseCharacterAnimInstance.h"

#include "Actors/Equipment/Weapons/RangedWeaponItem.h"
#include "Characters/XyzBaseCharacter.h"
#include "Characters/Controllers/XyzPlayerController.h"
#include "Components/CharacterComponents/CharacterAttributesComponent.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "Components/MovementComponents/XyzBaseCharMovementComponent.h"

void UXyzBaseCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	checkf(TryGetPawnOwner()->IsA<AXyzBaseCharacter>(), TEXT("UXyzBaseCharacterAnimInstance::NativeBeginPlay() should be used only with AXyzBaseCharacter"))
		CachedBaseCharacter = StaticCast<AXyzBaseCharacter*>(TryGetPawnOwner());
}

void UXyzBaseCharacterAnimInstance::NativeUpdateAnimation(const float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!CachedBaseCharacter.IsValid())
	{
		return;
	}

	const UXyzBaseCharMovementComponent* BaseCharMovementComponent = CachedBaseCharacter->GetBaseCharacterMovementComponent();
	bIsFalling = BaseCharMovementComponent->IsFalling();
	bIsCrouching = BaseCharMovementComponent->IsCrouching();
	bIsSprinting = BaseCharMovementComponent->IsSprinting();
	bIsProne = BaseCharMovementComponent->IsProne();
	bIsSwimming = BaseCharMovementComponent->IsSwimming();
	bIsOnLadder = BaseCharMovementComponent->IsOnLadder();
	bIsOnZipline = BaseCharMovementComponent->IsOnZipline();
	bIsWallRunning = BaseCharMovementComponent->IsWallRunning();
	bIsSliding = BaseCharMovementComponent->IsSliding();
	bIsOnTopOfCurrentLadder = BaseCharMovementComponent->IsOnTopOfCurrentLadder();
	bIsAiming = CachedBaseCharacter->IsAiming();
	bIsOutOfStamina = CachedBaseCharacter->GetCharacterAttributesComponent()->IsOutOfStamina();

	const UCharacterEquipmentComponent* CharacterEquipmentComponent = CachedBaseCharacter->GetCharacterEquipmentComponent();
	bIsPrimaryItemEquipped = CharacterEquipmentComponent->IsPrimaryItemEquipped();
	CurrentRangedWeaponType = CharacterEquipmentComponent->GetCurrentRangedWeaponType();
	Velocity = BaseCharMovementComponent->Velocity;
	MovementDirection = CalculateDirection(BaseCharMovementComponent->Velocity, CachedBaseCharacter->GetActorRotation());
	MovementSpeed = BaseCharMovementComponent->Velocity.Size();
	CurrentWallRunSide = BaseCharMovementComponent->GetCurrentWallRunSide();
	CharacterPitch = CachedBaseCharacter->GetActorRotation().Pitch;
	CameraPitch = CalculateCameraPitch();
	AimRotation = CachedBaseCharacter->GetAimOffset();
	if (bIsOnLadder)
	{
		LadderSpeedRatio = BaseCharMovementComponent->GetLadderSpeedRatio();
	}

	const float PelvisOffset = +CachedBaseCharacter->GetIKPelvisOffset();

	LeftFootEffectorLocation = FVector(CachedBaseCharacter->GetIKLeftFootOffset() + PelvisOffset, 0.f, 0.f);
	RightFootEffectorLocation = FVector(CachedBaseCharacter->GetIKRightFootOffset() + PelvisOffset, 0.f, 0.f);
	PelvisEffectorLocation = FVector(0.f, 0.f, PelvisOffset);
	const ARangedWeaponItem* CurrentRangedWeapon = CharacterEquipmentComponent->GetCurrentRangedWeapon();
	if (IsValid(CurrentRangedWeapon))
	{
		bIsReloading = CurrentRangedWeapon->IsReloading();
		FTransform ForeGripTransform = CurrentRangedWeapon->GetForeGripSocketTransform();
		const FVector ForeGripLocation = ForeGripTransform.GetLocation() - PelvisOffset * FVector::UpVector;
		ForeGripTransform.SetLocation(ForeGripLocation);
		CurrentRangedWeaponForeGripTransform = ForeGripTransform;
	}
}

float UXyzBaseCharacterAnimInstance::CalculateCameraPitch()
{
	const AXyzPlayerController* Controller = CachedBaseCharacter->GetController<AXyzPlayerController>();
	if (IsValid(Controller) && !Controller->IgnoresFPCameraPitch())
	{
		return Controller->GetControlRotation().Pitch;
	}
	return 0.f;
}
