// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/PlayerCharacter.h"

#include "Actors/Equipment/Weapons/RangedWeaponItem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/TimelineComponent.h"

#include "Components/MovementComponents/XyzBaseCharMovementComponent.h"
#include "Actors/Interactive/Environment/Ladder.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->bUsePawnControlRotation = false;

	Team = ETeam::Player;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(ProneCameraTimelineCurve))
	{
		FOnTimelineFloatStatic TimelineUpdate;
		TimelineUpdate.BindUObject(this, &APlayerCharacter::UpdateProneCameraTimeline);
		ProneCameraTimeline.AddInterpFloat(ProneCameraTimelineCurve, TimelineUpdate);
	}

	if (IsValid(AimingFOVTimelineCurve))
	{
		FOnTimelineFloatStatic TimelineUpdate;
		TimelineUpdate.BindUObject(this, &APlayerCharacter::UpdateAimingFOVTimeline);
		AimingFOVTimeline.AddInterpFloat(AimingFOVTimelineCurve, TimelineUpdate);
	}

	if (IsValid(WallRunCameraTimelineCurve))
	{
		FOnTimelineFloatStatic TimelineUpdate;
		TimelineUpdate.BindUObject(this, &APlayerCharacter::UpdateWallRunCameraTimeline);
		WallRunCameraTimeline.AddInterpFloat(WallRunCameraTimelineCurve, TimelineUpdate);
	}
}

void APlayerCharacter::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ProneCameraTimeline.TickTimeline(DeltaSeconds);
	AimingFOVTimeline.TickTimeline(DeltaSeconds);
	WallRunCameraTimeline.TickTimeline(DeltaSeconds);
}

void APlayerCharacter::OnMovementModeChanged(const EMovementMode PrevMovementMode, const uint8 PreviousCustomMode)
{
	if (BaseCharacterMovementComponent->IsSwimming())
	{
		if (BaseCharacterMovementComponent->IsProne())
		{
			bShouldSkipProneTimeline = true;
		}
	}
	else if (BaseCharacterMovementComponent->IsFalling() && PrevMovementMode == MOVE_Walking)
	{
		CachedSpringArmSocketOffset = SpringArmComponent->SocketOffset;
	}

	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
}

bool APlayerCharacter::IsCameraManagerValid()
{
	if (CachedCameraManager.IsValid())
	{
		return true;
	}

	const APlayerController* PlayerController = GetController<APlayerController>();
	if (IsValid(PlayerController))
	{
		CachedCameraManager = PlayerController->PlayerCameraManager;
		return true;
	}

	return false;
}

// General Movement

void APlayerCharacter::MoveForward(const float Value)
{
	Super::MoveForward(Value);
	if ((GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling()) && !FMath::IsNearlyZero(Value))
	{
		const FRotator Rotation(0.f, GetControlRotation().Yaw, 0.f);
		AddMovementInput(Rotation.Vector(), Value);
	}
}

void APlayerCharacter::MoveRight(const float Value)
{
	Super::MoveRight(Value);
	if ((GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling()) && !FMath::IsNearlyZero(Value))
	{
		const FRotator Rotation(0.f, GetControlRotation().Yaw, 0.f);
		const FVector RightVector = Rotation.RotateVector(FVector::RightVector);
		AddMovementInput(RightVector, Value);
	}
}

void APlayerCharacter::Turn(const float Value)
{
	Super::Turn(Value);
	AddControllerYawInput(Value * GetAimTurnModifier());
}

void APlayerCharacter::LookUp(const float Value)
{
	Super::LookUp(Value);
	AddControllerPitchInput(Value * GetAimLookUpModifier());
}

void APlayerCharacter::TurnAtRate(const float Value)
{
	Super::Turn(Value);
	AddControllerYawInput(Value * GetAimTurnModifier() * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::LookUpAtRate(const float Value)
{
	Super::LookUp(Value);
	AddControllerPitchInput(Value * GetAimLookUpModifier() * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

// Aiming

void APlayerCharacter::OnStartAimingInternal()
{
	Super::OnStartAimingInternal();

	if (IsCameraManagerValid())
	{
		const ARangedWeaponItem* RangedWeapon = CharacterEquipmentComponent->GetCurrentRangedWeapon();
		if (IsValid(RangedWeapon))
		{
			DefaultCameraFOV = CachedCameraManager->DefaultFOV;
			TargetAimingFOV = RangedWeapon->GetAimingFOV();

			if (IsValid(AimingFOVTimelineCurve))
			{
				AimingFOVTimeline.Play();
			}
			else
			{
				CachedCameraManager->SetFOV(TargetAimingFOV);
			}
		}
	}
}

void APlayerCharacter::OnStopAimingInternal()
{
	Super::OnStopAimingInternal();

	if (!IsCameraManagerValid())
	{
		return;
	}

	if (IsValid(AimingFOVTimelineCurve))
	{
		AimingFOVTimeline.Reverse();
	}
	else if (CachedCameraManager.IsValid())
	{
		CachedCameraManager->UnlockFOV();
	}
}

float APlayerCharacter::GetAimTurnModifier() const
{
	if (bIsAiming)
	{
		return AimTurnModifier;
	}

	return 1.f;
}

float APlayerCharacter::GetAimLookUpModifier() const
{
	if (bIsAiming)
	{
		return AimLookUpModifier;
	}

	return 1.f;
}

// Timelines

void APlayerCharacter::UpdateProneCameraTimeline(const float Value)
{
	SpringArmComponent->TargetOffset.Z = CachedSpringArmTargetOffset.Z + NewSpringArmTargetOffsetDelta.Z * Value;
	SpringArmComponent->SocketOffset = CachedSpringArmSocketOffset + NewSpringArmSocketOffsetDelta * Value;
}

void APlayerCharacter::UpdateAimingFOVTimeline(const float Value)
{
	if (CachedCameraManager.IsValid())
	{
		const float TargetFOV = TargetAimingFOV + (DefaultCameraFOV - TargetAimingFOV) * Value;
		CachedCameraManager->SetFOV(TargetFOV);
	}
}

void APlayerCharacter::UpdateWallRunCameraTimeline(const float Value)
{
	SpringArmComponent->SocketOffset.Y = CachedSpringArmSocketOffset.Y * Value;
}

// Jumping

void APlayerCharacter::Jump()
{
	if (BaseCharacterMovementComponent->IsProne())
	{
		bShouldSkipProneTimeline = true;
	}
	Super::Jump();
}

bool APlayerCharacter::CanJumpInternal_Implementation() const
{
	return !BaseCharacterMovementComponent->IsMantling() && (bIsCrouched || Super::CanJumpInternal_Implementation());
}

void APlayerCharacter::OnJumped_Implementation()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
}

// Swimming

void APlayerCharacter::SwimForward(const float Value)
{
	Super::SwimForward(Value);
	if (BaseCharacterMovementComponent->IsSwimming() && !FMath::IsNearlyZero(Value))
	{
		FRotator Rotation;
		if (BaseCharacterMovementComponent->IsSwimmingOnWaterPlane())
		{
			Rotation = FRotator(0.f, GetControlRotation().Yaw, 0.f);
		}
		else
		{
			Rotation = FRotator(GetControlRotation().Pitch, GetControlRotation().Yaw, 0.f);
		}
		AddMovementInput(Rotation.Vector(), Value);
	}
}

void APlayerCharacter::SwimRight(const float Value)
{
	Super::SwimRight(Value);
	if (BaseCharacterMovementComponent->IsSwimming() && !FMath::IsNearlyZero(Value))
	{
		const FRotator Rotation(0.f, GetControlRotation().Yaw, 0.f);
		const FVector RightVector = Rotation.RotateVector(FVector::RightVector);
		AddMovementInput(RightVector, Value);
	}
}

void APlayerCharacter::SwimUp(const float Value)
{
	Super::SwimUp(Value);
	BaseCharacterMovementComponent->SwimUp(Value);
}

void APlayerCharacter::Dive()
{
	Super::Dive();
	BaseCharacterMovementComponent->StartDive();
}

// Sliding

void APlayerCharacter::OnStartSlide(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust)
{
	Super::OnStartSlide(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SpringArmComponent->TargetOffset += FVector(0.f, 0.f, HalfHeightAdjust);
}

void APlayerCharacter::OnStopSlide(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust)
{
	Super::OnStopSlide(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SpringArmComponent->TargetOffset -= FVector(0.f, 0.f, HalfHeightAdjust);
}

// Crouching

bool APlayerCharacter::CanUnCrouch()
{
	return !ProneCameraTimeline.IsPlaying() && Super::CanUnCrouch();
}

void APlayerCharacter::OnStartCrouch(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SpringArmComponent->TargetOffset += FVector(0.f, 0.f, HalfHeightAdjust);
}

void APlayerCharacter::OnEndCrouch(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SpringArmComponent->TargetOffset -= FVector(0.f, 0.f, HalfHeightAdjust);
}

// Proning

bool APlayerCharacter::CanUnProne()
{
	return !ProneCameraTimeline.IsPlaying() && Super::CanUnProne();
}

void APlayerCharacter::OnStartProne(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust)
{
	Super::OnStartProne(HalfHeightAdjust, ScaledHalfHeightAdjust);

	CachedSpringArmTargetOffset = SpringArmComponent->TargetOffset;
	CachedSpringArmSocketOffset = SpringArmComponent->SocketOffset;
	NewSpringArmTargetOffsetDelta = FVector(0.f, 0.f, HalfHeightAdjust + ProneCameraHeightOffset);
	NewSpringArmSocketOffsetDelta = FVector(ProneCameraProximityOffset, ProneCameraRightOffset, 0.f);

	if (IsValid(ProneCameraTimelineCurve))
	{
		ProneCameraTimeline.Play();
	}
	else
	{
		SpringArmComponent->TargetOffset += NewSpringArmTargetOffsetDelta;
		SpringArmComponent->SocketOffset += NewSpringArmSocketOffsetDelta;
	}
}

void APlayerCharacter::OnEndProne(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust)
{
	Super::OnEndProne(HalfHeightAdjust, ScaledHalfHeightAdjust);

	if (!bShouldSkipProneTimeline && IsValid(ProneCameraTimelineCurve))
	{
		NewSpringArmTargetOffsetDelta = FVector(0.f, 0.f, HalfHeightAdjust + ProneCameraHeightOffset);
		ProneCameraTimeline.Reverse();
	}
	else
	{
		SpringArmComponent->TargetOffset = CachedSpringArmTargetOffset;
		SpringArmComponent->SocketOffset = CachedSpringArmSocketOffset;
	}
	bShouldSkipProneTimeline = false;
}

// Interactive Actors

void APlayerCharacter::ClimbLadderUp(const float Value)
{
	Super::ClimbLadderUp(Value);
	if (BaseCharacterMovementComponent->IsOnLadder() && !FMath::IsNearlyZero(Value))
	{
		const FVector LadderUpVector = BaseCharacterMovementComponent->GetCurrentLadder()->GetActorUpVector();
		AddMovementInput(LadderUpVector, Value);
	}
}

void APlayerCharacter::OnWallRunStart()
{
	Super::OnWallRunStart();

	if (IsValid(WallRunCameraTimelineCurve))
	{
		WallRunCameraTimeline.Play();
	}
	else
	{
		SpringArmComponent->SocketOffset.Y = 0.f;
	}
}

void APlayerCharacter::OnWallRunEnd()
{
	Super::OnWallRunEnd();

	if (IsValid(WallRunCameraTimelineCurve))
	{
		WallRunCameraTimeline.Reverse();
	}
	else
	{
		SpringArmComponent->SocketOffset.Y = CachedSpringArmSocketOffset.Y;
	}
}
