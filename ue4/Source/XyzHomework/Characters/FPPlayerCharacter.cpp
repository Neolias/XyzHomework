// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/FPPlayerCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Components/MovementComponents/XyzBaseCharMovementComponent.h"
#include "Components/CharacterComponents/CharacterAttributesComponent.h"
#include "Actors/Interactive/Environment/Ladder.h"
#include "Actors/Interactive/Environment/Zipline.h"
#include "Controllers/XyzPlayerController.h"

AFPPlayerCharacter::AFPPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	FPMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPSkeletalMesh"));
	FPMeshComponent->SetupAttachment(GetCapsuleComponent());
	FPMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -86.f));
	FPMeshComponent->CastShadow = false;
	FPMeshComponent->bCastDynamicShadow = false;
	FPMeshComponent->SetOnlyOwnerSee(true);

	FPCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FPCamera"));
	FPCameraComponent->SetupAttachment(FPMeshComponent, CameraSocket);
	FPCameraComponent->bUsePawnControlRotation = true;

	if (IsValid(SkeletalMeshComponent))
	{
		SkeletalMeshComponent->SetOwnerNoSee(true);
		SkeletalMeshComponent->bCastHiddenShadow = true;
	}
	CameraComponent->bAutoActivate = false;
	SpringArmComponent->bAutoActivate = false;
	SpringArmComponent->bUsePawnControlRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = true;

	bIsFirstPerson = true;
}

void AFPPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(WallRunCameraTiltCurve))
	{
		FOnTimelineFloatStatic TimelineCallBack;
		TimelineCallBack.BindUObject(this, &AFPPlayerCharacter::UpdateWallRunCameraTilt);
		WallRunCameraTiltTimeline.AddInterpFloat(WallRunCameraTiltCurve, TimelineCallBack);
	}
}

void AFPPlayerCharacter::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateCameraAlignment(DeltaSeconds);

	WallRunCameraTiltTimeline.TickTimeline(DeltaSeconds);
}

// General

FRotator AFPPlayerCharacter::GetViewRotation() const
{
	FRotator Result = Super::GetViewRotation();

	// Makes camera alignment smoother
	if (IsAligningFPCameraToSocketRotation())
	{
		FRotator SocketRotation = FPMeshComponent->GetSocketRotation(CameraSocket);
		if (CharacterAttributesComponent->IsOutOfStamina())
		{
			SocketRotation.Yaw = Result.Yaw;
		}
		Result = SocketRotation;
	}

	return Result;
}

bool AFPPlayerCharacter::IsFPMontagePlaying() const
{
	const UAnimInstance* FPAnimInstance = FPMeshComponent->GetAnimInstance();
	return IsValid(FPAnimInstance) && FPAnimInstance->IsAnyMontagePlaying();
}

void AFPPlayerCharacter::OnMovementModeChanged(const EMovementMode PrevMovementMode, const uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (BaseCharacterMovementComponent->IsOnLadder())
	{
		bWantsToAttachToLadder = true;
		const ALadder* CurrentLadder = BaseCharacterMovementComponent->GetCurrentLadder();
		if (IsValid(CurrentLadder))
		{
			FRotator ForcedRotation = BaseCharacterMovementComponent->GetCurrentLadder()->GetActorForwardVector().ToOrientationRotator();
			ForcedRotation.Yaw += 180.f;
			ForcedTargetControlRotation = ForcedRotation;
			StartFPCameraAlignment();
		}
	}
	else if (BaseCharacterMovementComponent->IsOnZipline())
	{
		OnAttachedToZipline();
	}
	else if (BaseCharacterMovementComponent->IsSwimming())
	{
		if (XyzPlayerController.IsValid())
		{
			XyzPlayerController->ShouldIgnoreFPCameraPitch(true);
		}
	}
	else if (BaseCharacterMovementComponent->IsWallRunning())
	{
		if (XyzPlayerController.IsValid())
		{
			XyzPlayerController->ShouldIgnoreFPCameraPitch(true);
			XyzPlayerController->SetIgnoreLookInput(true);
			XyzPlayerController->SetIgnoreMoveInput(true);
		}
		StartWallRunCameraTilt();
	}
	else if (PrevMovementMode == MOVE_Swimming)
	{
		if (XyzPlayerController.IsValid())
		{
			XyzPlayerController->ShouldIgnoreFPCameraPitch(false);
		}
	}
	else if (PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Ladder)
	{
		OnDetachedFromLadder();
	}
	else if (PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Zipline)
	{
		OnDetachedFromZipline();
	}
	else if (PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Mantling)
	{
		bWantsToEndMantle = true;
	}
	else if (PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_WallRun)
	{
		if (XyzPlayerController.IsValid())
		{
			XyzPlayerController->ShouldIgnoreFPCameraPitch(false);
			XyzPlayerController->SetIgnoreLookInput(false);
			XyzPlayerController->SetIgnoreMoveInput(false);
		}
		EndWallRunCameraTilt();
	}
}

// Camera

bool AFPPlayerCharacter::IsAligningFPCameraToSocketRotation() const
{
	return IsFPMontagePlaying() && (BaseCharacterMovementComponent->IsMantling() || BaseCharacterMovementComponent->IsOnTopOfCurrentLadder()) || CharacterAttributesComponent->IsOutOfStamina() || bIsHardLanding;
}

void AFPPlayerCharacter::StartFPCameraAlignment()
{
	if (XyzPlayerController.IsValid())
	{
		XyzPlayerController->SetIgnoreLookInput(true);
		XyzPlayerController->SetIgnoreMoveInput(true);
	}
	bIsForcedToAlignFPCamera = true;
}

void AFPPlayerCharacter::EndFPCameraAlignment()
{
	bIsForcedToAlignFPCamera = false;
	if (XyzPlayerController.IsValid())
	{
		XyzPlayerController->SetIgnoreLookInput(false);
		XyzPlayerController->SetIgnoreMoveInput(false);
	}
}

void AFPPlayerCharacter::UpdateCameraAlignment(const float DeltaSeconds)
{
	if (!XyzPlayerController.IsValid())
	{
		return;
	}

	if (IsAligningFPCameraToSocketRotation())
	{
		FRotator TargetControlRotation = XyzPlayerController->GetControlRotation();
		FRotator SocketRotation = FPMeshComponent->GetSocketRotation(CameraSocket);
		if (CharacterAttributesComponent->IsOutOfStamina())
		{
			SocketRotation.Yaw = TargetControlRotation.Yaw;
		}
		TargetControlRotation = FMath::RInterpTo(TargetControlRotation, SocketRotation, DeltaSeconds, AnimMontageCameraBlendSpeed);
		XyzPlayerController->SetControlRotation(TargetControlRotation);
	}

	if (BaseCharacterMovementComponent->IsWallRunning())
	{
		if (BaseCharacterMovementComponent->IsWallRunning() && XyzPlayerController.IsValid())
		{
			const FRotator CurrentWallRunDirection = BaseCharacterMovementComponent->GetCurrentWallRunDirection().ToOrientationRotator();
			FRotator TargetControlRotation = XyzPlayerController->GetControlRotation();
			TargetControlRotation.Pitch = CurrentWallRunDirection.Pitch;
			TargetControlRotation.Yaw = CurrentWallRunDirection.Yaw;
			TargetControlRotation = FMath::RInterpTo(XyzPlayerController->GetControlRotation(), TargetControlRotation, DeltaSeconds, AlignmentBlendSpeed);
			XyzPlayerController->SetControlRotation(TargetControlRotation);
		}
	}

	if (bIsForcedToAlignFPCamera)
	{
		FRotator TargetControlRotation = XyzPlayerController->GetControlRotation();
		if (!TargetControlRotation.Equals(ForcedTargetControlRotation, 0.1f))
		{
			TargetControlRotation = FMath::RInterpTo(TargetControlRotation, ForcedTargetControlRotation, DeltaSeconds, AlignmentBlendSpeed);
			XyzPlayerController->SetControlRotation(TargetControlRotation);
		}
		else
		{
			EndFPCameraAlignment();

			if (bWantsToEndMantle)
			{
				OnEndMantle();
			}
			else if (bWantsToAttachToLadder)
			{
				OnAttachedToLadder();
			}
			else if (bWantsToEndOutOfStamina)
			{
				bUseControllerRotationYaw = true;
				bWantsToEndOutOfStamina = false;
			}
			else if (bWantsToEndHardLand)
			{
				bUseControllerRotationYaw = true;
				bWantsToEndHardLand = false;
			}
		}
	}
}

// Landing

void AFPPlayerCharacter::OnHardLandStart()
{
	Super::OnHardLandStart();

	if (!IsValid(HardLandFPAnimMontage))
	{
		return;
	}

	bUseControllerRotationYaw = false;
	UAnimInstance* FPAnimInstance = FPMeshComponent->GetAnimInstance();
	if (IsValid(FPAnimInstance))
	{
		if (XyzPlayerController.IsValid())
		{
			XyzPlayerController->ShouldIgnoreFPCameraPitch(true);
		}
		FPAnimInstance->Montage_Play(HardLandFPAnimMontage);
	}
}

void AFPPlayerCharacter::OnHardLandEnd()
{
	Super::OnHardLandEnd();
	if (XyzPlayerController.IsValid())
	{
		XyzPlayerController->ShouldIgnoreFPCameraPitch(false);
	}
	ForcedTargetControlRotation = GetActorForwardVector().ToOrientationRotator();
	bWantsToEndHardLand = true;
	StartFPCameraAlignment();
}

// Sliding

void AFPPlayerCharacter::OnStartSlide(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust)
{
	Super::OnStartSlide(HalfHeightAdjust, ScaledHalfHeightAdjust);

	if (!IsValid(SlideFPAnimMontage))
	{
		return;
	}

	FVector& RelativeMeshLocation = FPMeshComponent->GetRelativeLocation_DirectMutable();
	RelativeMeshLocation.Z += HalfHeightAdjust;
	bUseControllerRotationYaw = false;
	UAnimInstance* FPAnimInstance = FPMeshComponent->GetAnimInstance();
	if (IsValid(FPAnimInstance))
	{
		bUseControllerRotationYaw = false;
		if (XyzPlayerController.IsValid())
		{
			XyzPlayerController->ShouldIgnoreFPCameraPitch(true);
			XyzPlayerController->SetIgnoreLookInput(true);
			XyzPlayerController->SetIgnoreMoveInput(true);
		}
		ForcedTargetControlRotation = GetActorForwardVector().ToOrientationRotator();
		const float Duration = FPAnimInstance->Montage_Play(SlideFPAnimMontage, 1.f, EMontagePlayReturnType::Duration);
		// Uncomment to align camera after sliding
		//GetWorldTimerManager().SetTimer(FPSlideTimer, this, &AFPPlayerCharacter::StartFPCameraAlignment, Duration, false);
	}
}

void AFPPlayerCharacter::OnStopSlide(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust)
{
	Super::OnStopSlide(HalfHeightAdjust, ScaledHalfHeightAdjust);
	bUseControllerRotationYaw = true;
	if (XyzPlayerController.IsValid())
	{
		XyzPlayerController->ShouldIgnoreFPCameraPitch(false);
		XyzPlayerController->SetIgnoreLookInput(false);
		XyzPlayerController->SetIgnoreMoveInput(false);
	}
	FVector& RelativeMeshLocation = FPMeshComponent->GetRelativeLocation_DirectMutable();
	RelativeMeshLocation.Z -= HalfHeightAdjust;
}

// OutOfStamina

void AFPPlayerCharacter::OnOutOfStaminaStart()
{
	Super::OnOutOfStaminaStart();
	if (XyzPlayerController.IsValid())
	{
		XyzPlayerController->ShouldIgnoreFPCameraPitch(true);
	}
}

void AFPPlayerCharacter::OnOutOfStaminaEnd()
{
	Super::OnOutOfStaminaEnd();
	bUseControllerRotationYaw = false;
	if (XyzPlayerController.IsValid())
	{
		XyzPlayerController->ShouldIgnoreFPCameraPitch(false);
	}
	ForcedTargetControlRotation = GetActorForwardVector().ToOrientationRotator();
	bWantsToEndOutOfStamina = true;
	StartFPCameraAlignment();
}

// Crouching / Proning

void AFPPlayerCharacter::OnStartCrouch(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	if (XyzPlayerController.IsValid())
	{
		XyzPlayerController->ShouldIgnoreFPCameraPitch(true);
	}
	const AFPPlayerCharacter* DefaultCharacter = GetDefault<AFPPlayerCharacter>(GetClass());
	FVector& RelativeMeshLocation = FPMeshComponent->GetRelativeLocation_DirectMutable();
	RelativeMeshLocation.Z = DefaultCharacter->FPMeshComponent->GetRelativeLocation().Z + HalfHeightAdjust;
}

void AFPPlayerCharacter::OnEndCrouch(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	if (XyzPlayerController.IsValid())
	{
		XyzPlayerController->ShouldIgnoreFPCameraPitch(false);
	}
	const AFPPlayerCharacter* DefaultCharacter = GetDefault<AFPPlayerCharacter>(GetClass());
	FVector& RelativeMeshLocation = FPMeshComponent->GetRelativeLocation_DirectMutable();
	RelativeMeshLocation.Z = DefaultCharacter->FPMeshComponent->GetRelativeLocation().Z;
}

void AFPPlayerCharacter::OnStartProne(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust)
{
	Super::OnStartProne(HalfHeightAdjust, ScaledHalfHeightAdjust);
	if (XyzPlayerController.IsValid())
	{
		XyzPlayerController->ShouldIgnoreFPCameraPitch(true);
	}
	FVector& RelativeMeshLocation = FPMeshComponent->GetRelativeLocation_DirectMutable();
	RelativeMeshLocation.Z += HalfHeightAdjust;
}

void AFPPlayerCharacter::OnEndProne(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust)
{
	Super::OnEndProne(HalfHeightAdjust, ScaledHalfHeightAdjust);
	if (XyzPlayerController.IsValid())
	{
		XyzPlayerController->ShouldIgnoreFPCameraPitch(false);
	}
	FVector& RelativeMeshLocation = FPMeshComponent->GetRelativeLocation_DirectMutable();
	RelativeMeshLocation.Z -= HalfHeightAdjust;
}

// Mantling

void AFPPlayerCharacter::OnMantle(const FMantlingSettings& MantlingSettings, const FMantlingMovementParameters& MantlingParameters)
{
	Super::OnMantle(MantlingSettings, MantlingParameters);
	UAnimInstance* FPAnimInstance = FPMeshComponent->GetAnimInstance();
	if (IsValid(FPAnimInstance) && IsValid(MantlingSettings.FPMantlingMontage))
	{
		bUseControllerRotationYaw = false;
		if (XyzPlayerController.IsValid())
		{
			XyzPlayerController->ShouldIgnoreFPCameraPitch(true);
			XyzPlayerController->SetIgnoreLookInput(true);
			XyzPlayerController->SetIgnoreMoveInput(true);
		}
		ForcedTargetControlRotation = MantlingParameters.TargetRotation;
		const float MantlingDuration = FPAnimInstance->Montage_Play(MantlingSettings.FPMantlingMontage, 1.f, EMontagePlayReturnType::Duration, MantlingParameters.StartTime);
		GetWorldTimerManager().SetTimer(FPMantlingTimer, this, &AFPPlayerCharacter::StartFPCameraAlignment, MantlingDuration, false);
	}
}

void AFPPlayerCharacter::OnEndMantle()
{
	if (XyzPlayerController.IsValid())
	{
		XyzPlayerController->ShouldIgnoreFPCameraPitch(false);
		XyzPlayerController->SetIgnoreLookInput(false);
		XyzPlayerController->SetIgnoreMoveInput(false);
	}
	bUseControllerRotationYaw = true;
	bWantsToEndMantle = false;
}

// Interactive Actors

void AFPPlayerCharacter::OnAttachedToLadder()
{
	bUseControllerRotationYaw = false;
	if (XyzPlayerController.IsValid())
	{
		XyzPlayerController->ShouldIgnoreFPCameraPitch(true);
		const FRotator CurrentControlRotation = XyzPlayerController->GetControlRotation();
		APlayerCameraManager* CameraManager = XyzPlayerController->PlayerCameraManager;
		CameraManager->ViewPitchMin = LadderCameraMinPitch;
		CameraManager->ViewPitchMax = LadderCameraMaxPitch;
		CameraManager->ViewYawMin = CurrentControlRotation.Yaw + LadderCameraMinYaw;
		CameraManager->ViewYawMax = CurrentControlRotation.Yaw + LadderCameraMaxYaw;
	}
	bWantsToAttachToLadder = false;
}

void AFPPlayerCharacter::OnDetachedFromLadder()
{
	bUseControllerRotationYaw = true;
	if (XyzPlayerController.IsValid())
	{
		XyzPlayerController->ShouldIgnoreFPCameraPitch(true);
		APlayerCameraManager* CameraManager = XyzPlayerController->PlayerCameraManager;
		const APlayerCameraManager* DefaultCameraManager = CameraManager->GetClass()->GetDefaultObject<APlayerCameraManager>();

		CameraManager->ViewPitchMin = DefaultCameraManager->ViewPitchMin;
		CameraManager->ViewPitchMax = DefaultCameraManager->ViewPitchMax;
		CameraManager->ViewYawMin = DefaultCameraManager->ViewYawMin;
		CameraManager->ViewYawMax = DefaultCameraManager->ViewYawMax;
	}
}

void AFPPlayerCharacter::OnAttachedToLadderFromTop(ALadder* Ladder)
{
	Super::OnAttachedToLadderFromTop(Ladder);

	UAnimInstance* FPAnimInstance = FPMeshComponent->GetAnimInstance();
	if (IsValid(FPAnimInstance))
	{
		const float Duration = FPAnimInstance->Montage_Play(Ladder->GetAttachFromTopFPAnimMontage(), 1.f, EMontagePlayReturnType::Duration);
		GetWorldTimerManager().SetTimer(FPLadderTimer, this, &AFPPlayerCharacter::StartFPCameraAlignment, Duration, false);
	}
}

void AFPPlayerCharacter::OnAttachedToZipline()
{
	bUseControllerRotationYaw = false;
	const FRotator ZiplineRotation = BaseCharacterMovementComponent->GetCurrentZipline()->GetZiplineSpanVector().ToOrientationRotator();
	if (XyzPlayerController.IsValid())
	{
		XyzPlayerController->ShouldIgnoreFPCameraPitch(true);
		APlayerCameraManager* CameraManager = XyzPlayerController->PlayerCameraManager;
		CameraManager->ViewPitchMin = ZiplineCameraMinPitch;
		CameraManager->ViewPitchMax = ZiplineCameraMaxPitch;
		CameraManager->ViewYawMin = ZiplineRotation.Yaw + ZiplineCameraMinYaw;
		CameraManager->ViewYawMax = ZiplineRotation.Yaw + ZiplineCameraMaxYaw;
	}
	ForcedTargetControlRotation = ZiplineRotation;
	StartFPCameraAlignment();
}

void AFPPlayerCharacter::OnDetachedFromZipline()
{
	bUseControllerRotationYaw = true;
	if (XyzPlayerController.IsValid())
	{
		XyzPlayerController->ShouldIgnoreFPCameraPitch(false);
		APlayerCameraManager* CameraManager = XyzPlayerController->PlayerCameraManager;
		const APlayerCameraManager* DefaultCameraManager = CameraManager->GetClass()->GetDefaultObject<APlayerCameraManager>();

		CameraManager->ViewPitchMin = DefaultCameraManager->ViewPitchMin;
		CameraManager->ViewPitchMax = DefaultCameraManager->ViewPitchMax;
		CameraManager->ViewYawMin = DefaultCameraManager->ViewYawMin;
		CameraManager->ViewYawMax = DefaultCameraManager->ViewYawMax;
	}
}

// Wall Running

void AFPPlayerCharacter::UpdateWallRunCameraTilt(const float Value) const
{
	if (XyzPlayerController.IsValid())
	{
		FRotator TargetControlRotation = XyzPlayerController->GetControlRotation();
		TargetControlRotation.Roll = BaseCharacterMovementComponent->GetCurrentWallRunSide() == EWallRunSide::Left ? Value : -Value;
		XyzPlayerController->SetControlRotation(TargetControlRotation);
	}
}

// Death

void AFPPlayerCharacter::OnDeathStarted()
{
	Super::OnDeathStarted();

	FPCameraComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	if (IsValid(SkeletalMeshComponent))
	{
		SkeletalMeshComponent->SetOwnerNoSee(false);
	}
	FPMeshComponent->SetOwnerNoSee(true);
}
