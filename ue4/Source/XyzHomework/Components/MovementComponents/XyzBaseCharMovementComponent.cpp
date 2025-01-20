// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/MovementComponents/XyzBaseCharMovementComponent.h"

#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PhysicsVolume.h"
#include "Curves/CurveVector.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ProfilingDebugging/CookStats.h"

#include "XyzHomeworkTypes.h"
#include "Actors/Equipment/Weapons/RangedWeaponItem.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterAttributesComponent.h"
#include "Actors/Interactive/Environment/Ladder.h"
#include "Actors/Interactive/Environment/Zipline.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

void UXyzBaseCharMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	checkf(Owner->IsA<AXyzBaseCharacter>(), TEXT("UXyzBaseCharMovementComponent::BeginPlay() should be used only with AXyzBaseCharacter"))
		BaseCharacter = StaticCast<AXyzBaseCharacter*>(Owner);
	if (IsValid(BaseCharacter))
	{
		CharacterAttributesComponent = BaseCharacter->GetCharacterAttributesComponent();
		CharacterEquipmentComponent = BaseCharacter->GetCharacterEquipmentComponent();
	}
}

// Network

FNetworkPredictionData_Client* UXyzBaseCharMovementComponent::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		UXyzBaseCharMovementComponent* MutableThis = const_cast<UXyzBaseCharMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_XyzCharacter(*this);
	}

	return Super::GetPredictionData_Client();
}

void UXyzBaseCharMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	//  FLAG_JumpPressed = 0x01,	// Jump pressed
	//	FLAG_WantsToCrouch = 0x02,	// Wants to crouch
	//	FLAG_Reserved_1 = 0x04,	// Reserved for future use
	//	FLAG_Reserved_2 = 0x08,	// Reserved for future use
	//	// Remaining bit masks are available for custom flags.
	//	FLAG_Custom_0 = 0x10, // Sprinting
	//	FLAG_Custom_1 = 0x20, // Mantling
	//	FLAG_Custom_2 = 0x40, // PressingSlide
	//	FLAG_Custom_3 = 0x80,

	if (BaseCharacter->GetLocalRole() == ROLE_Authority)
	{
		if (Flags & FSavedMove_Character::FLAG_Custom_0)
		{
			StartSprint();
		}
		else
		{
			StopSprint();
		}

		if (Flags & FSavedMove_Character::FLAG_Custom_1 && !IsMantling())
		{
			BaseCharacter->Mantle(true);
		}

		if (Flags & FSavedMove_Character::FLAG_Custom_2 && !bIsSliding)
		{
			StartSlide();
		}
	}

	Super::UpdateFromCompressedFlags(Flags);
}

// General

void UXyzBaseCharMovementComponent::OnMovementModeChanged(const EMovementMode PreviousMovementMode, const uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	switch (MovementMode)
	{
	case MOVE_Swimming:
		if (IsValid(CharacterEquipmentComponent) && CharacterEquipmentComponent->IsMeleeAttackActive())
		{
			CharacterEquipmentComponent->SetIsMeleeAttackActive(false);
		}
		if (IsCrouching())
		{
			if (IsProne())
			{
				UnProne();
				bWantsToProne = false;
			}
			UnCrouch();
			bWantsToCrouch = false;
		}
		BaseCharacter->GetCapsuleComponent()->SetCapsuleSize(SwimmingCapsuleRadius, SwimmingCapsuleHalfHeight);
		break;
	case MOVE_Walking:
		ForceTargetRotation = FRotator::ZeroRotator;
		bForceRotation = false;
		CurrentWallRunSide = EWallRunSide::None;

		if (PreviousMovementMode == MOVE_Falling)
		{
			bCanUseSameWallRunSide = false;
		}
		break;
	case MOVE_Falling:
		bNotifyApex = true;
		break;
	case MOVE_Custom:
		if (CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Mantling)
		{
			GetWorld()->GetTimerManager().SetTimer(MantlingTimer, this, &UXyzBaseCharMovementComponent::EndMantle, CurrentMantlingParameters.Duration, false);
		}
		break;
	default:
		break;
	}

	if (PreviousMovementMode == MOVE_Swimming)
	{
		const ACharacter* DefaultCharacter = BaseCharacter->GetClass()->GetDefaultObject<ACharacter>();
		BaseCharacter->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), true);
		bIsSwimmingOnWaterPlane = false;
		bIsDiving = false;
	}
}

void UXyzBaseCharMovementComponent::UpdateCharacterStateBeforeMovement(const float DeltaSeconds)
{
	if (bIsProne && (!bWantsToProne || !CanProneInCurrentState()))
	{
		UnProne();
	}
	else if (!bIsProne && bWantsToProne && CanProneInCurrentState())
	{
		Prone();
	}
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

float UXyzBaseCharMovementComponent::GetMaxSpeed() const
{
	float Result = Super::GetMaxSpeed();

	if (IsValid(CharacterAttributesComponent) && CharacterAttributesComponent->IsOutOfStamina())
	{
		Result = OutOfStaminaSpeed;
	}
	else if (bIsProne)
	{
		Result = ProneSpeed;
	}
	else if (IsOnLadder())
	{
		Result = ClimbingOnLadderMaxSpeed;
	}
	else if (IsOnZipline())
	{
		Result = ZiplineMovementMaxSpeed;
	}
	else if (IsWallRunning())
	{
		Result = WallRunMaxSpeed;
	}
	else if (IsValid(CharacterEquipmentComponent) && CharacterEquipmentComponent->IsThrowingItem())
	{
		Result = BaseCharacter->GetCurrentThrowItemMovementSpeed();
	}
	else if (IsValid(CharacterEquipmentComponent) && CharacterEquipmentComponent->IsReloadingWeapon())
	{
		Result = BaseCharacter->GetCurrentReloadingWalkSpeed();
	}
	else if (IsValid(BaseCharacter) && BaseCharacter->IsAiming())
	{
		Result = BaseCharacter->GetCurrentAimingMovementSpeed();
	}
	else if (bIsSprinting)
	{
		Result = SprintSpeed;
	}
	else if (bIsSliding)
	{
		Result = SlideSpeed;
	}

	return Result;
}

void UXyzBaseCharMovementComponent::PhysicsRotation(const float DeltaTime)
{
	if (bForceRotation)
	{
		const FRotator CurrentRotation = UpdatedComponent->GetComponentRotation();
		CurrentRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): CurrentRotation"));

		const FRotator DeltaRot = GetDeltaRotation(DeltaTime);
		DeltaRot.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): GetDeltaRotation"));

		constexpr float AngleTolerance = 1e-3f;

		if (!CurrentRotation.Equals(ForceTargetRotation, AngleTolerance))
		{
			FRotator DesiredRotation = ForceTargetRotation;

			if (!FMath::IsNearlyEqual(CurrentRotation.Pitch, DesiredRotation.Pitch, AngleTolerance))
			{
				DesiredRotation.Pitch = FMath::FixedTurn(CurrentRotation.Pitch, DesiredRotation.Pitch, DeltaRot.Pitch);
			}

			if (!FMath::IsNearlyEqual(CurrentRotation.Yaw, DesiredRotation.Yaw, AngleTolerance))
			{
				DesiredRotation.Yaw = FMath::FixedTurn(CurrentRotation.Yaw, DesiredRotation.Yaw, DeltaRot.Yaw);
			}

			if (!FMath::IsNearlyEqual(CurrentRotation.Roll, DesiredRotation.Roll, AngleTolerance))
			{
				DesiredRotation.Roll = FMath::FixedTurn(CurrentRotation.Roll, DesiredRotation.Roll, DeltaRot.Roll);
			}

			DesiredRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): DesiredRotation"));
			MoveUpdatedComponent(FVector::ZeroVector, DesiredRotation, false);
		}
		else
		{
			ForceTargetRotation = FRotator::ZeroRotator;
			bForceRotation = false;
		}
	}

	if (IsOnLadder())
	{
		return;
	}

	Super::PhysicsRotation(DeltaTime);
}

void UXyzBaseCharMovementComponent::PhysCustom(const float DeltaTime, const int32 Iterations)
{
	if (BaseCharacter->GetLocalRole() == ROLE_SimulatedProxy)
	{
		return;
	}

	switch (CustomMovementMode)
	{
	case (uint8)ECustomMovementMode::CMOVE_Mantling:
	{
		PhysMantling(DeltaTime, Iterations);
		break;
	}
	case (uint8)ECustomMovementMode::CMOVE_Ladder:
	{
		PhysLadder(DeltaTime, Iterations);
		break;
	}
	case (uint8)ECustomMovementMode::CMOVE_Zipline:
	{
		PhysZipline(DeltaTime, Iterations);
		break;
	}
	case (uint8)ECustomMovementMode::CMOVE_WallRun:
	{
		PhysWallRun(DeltaTime, Iterations);
		break;
	}
	default:
		break;
	}

	Super::PhysCustom(DeltaTime, Iterations);
}

// Swimming

bool UXyzBaseCharMovementComponent::IsSwimmingUnderWater(const FVector Delta) const
{
	if (Delta == FVector::ZeroVector && bIsSwimmingOnWaterPlane)
	{
		return false;
	}

	const APhysicsVolume* Volume = GetPhysicsVolume();
	const float VolumeTopPlane = Volume->GetActorLocation().Z + Volume->GetBounds().BoxExtent.Z * Volume->GetActorScale3D().Z;
	const float HeadPositionZ = BaseCharacter->GetMesh()->GetSocketLocation(HeadBoneName).Z;
	if (HeadPositionZ + Delta.Z < VolumeTopPlane)
	{
		return true;
	}
	return false;
}

bool UXyzBaseCharMovementComponent::IsSwimmingOnWaterPlane() const
{
	return IsSwimming() && bIsSwimmingOnWaterPlane;
}

void UXyzBaseCharMovementComponent::SwimUp(const float Value)
{
	if (!IsSwimming() || bIsDiving || FMath::IsNearlyZero(Value))
	{
		bIsSwimmingUp = false;
		TargetSwimUpSpeed = Velocity.Z;
	}
	else
	{
		bIsSwimmingUp = true;
		TargetSwimUpSpeed = GetMaxSpeed() * Value;
	}
}

void UXyzBaseCharMovementComponent::StartDive()
{
	if (bIsSwimmingOnWaterPlane)
	{
		bIsDiving = true;
		GetWorld()->GetTimerManager().SetTimer(DiveTimer, this, &UXyzBaseCharMovementComponent::StopDive, DiveActionLength, false);
	}
}

void UXyzBaseCharMovementComponent::StopDive()
{
	bIsDiving = false;
}

void UXyzBaseCharMovementComponent::PhysSwimming(const float DeltaTime, const int32 Iterations)
{
	Super::PhysSwimming(DeltaTime, Iterations);

	if (bIsDiving)
	{
		Velocity.Z = -DiveSpeed;
		const FVector Delta = Velocity * DeltaTime;
		if (IsSwimmingUnderWater(Delta))
		{
			bIsSwimmingOnWaterPlane = false;
		}
	}
	else if (bIsSwimmingOnWaterPlane)
	{
		const FVector CurrentLocation = BaseCharacter->GetActorLocation();
		const FVector WaterPlaneLocation = FindWaterLine(CurrentLocation, CurrentLocation + BaseCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * FVector::UpVector);
		const FVector Delta = (WaterPlaneLocation - CurrentLocation) * DeltaTime;
		FHitResult Hit;
		Velocity.Z = 0.f;
		SafeMoveUpdatedComponent(Delta, BaseCharacter->GetActorRotation(), false, Hit);
	}
	else if (IsSwimmingUnderWater())
	{
		const FVector Delta = Velocity * DeltaTime;
		if (!IsSwimmingUnderWater(Delta + WaterPlaneDetectionOffset))
		{
			bIsSwimmingOnWaterPlane = true;
		}

		if (bIsSwimmingUp)
		{
			FVector TargetVelocity = Velocity;
			TargetVelocity.Z = TargetSwimUpSpeed;
			Velocity = TargetVelocity.GetSafeNormal() * GetMaxSpeed();
			const FRotator TargetRotation = FMath::RInterpTo(BaseCharacter->GetActorRotation(), Velocity.ToOrientationRotator(), DeltaTime, SwimPitchRotationInterpSpeed);
			MoveUpdatedComponent(FVector::ZeroVector, TargetRotation, false);
		}
	}

	GEngine->AddOnScreenDebugMessage(11, 1.f, FColor::Yellow, FString::Printf(TEXT("bIsSwimmingOnWaterPlane: %hhd"), bIsSwimmingOnWaterPlane));
}

// Jumping

bool UXyzBaseCharMovementComponent::CanAttemptJump() const
{
	return !bIsSliding && !CharacterAttributesComponent->IsOutOfStamina() && !CharacterEquipmentComponent->IsMeleeAttackActive() && Super::CanAttemptJump();
}

bool UXyzBaseCharMovementComponent::DoJump(const bool bReplayingMoves)
{
	if (IsMovingOnGround())
	{
		const float NewStamina = FMath::Clamp(CharacterAttributesComponent->GetCurrentStamina() - StaminaConsumptionPerJump, 0.f, CharacterAttributesComponent->GetMaxStamina());
		CharacterAttributesComponent->SetCurrentStamina(NewStamina);
	}

	return Super::DoJump(bReplayingMoves);
}

// Sprinting

void UXyzBaseCharMovementComponent::StartSprint()
{
	if (bIsSprinting)
	{
		return;
	}

	bIsSprinting = true;
	bForceMaxAccel = 1;
	BaseCharacter->OnSprintStart();

	if (BaseCharacter->GetLocalRole() == ROLE_Authority)
	{
		Multicast_StartSprint();
	}
}

void UXyzBaseCharMovementComponent::Multicast_StartSprint_Implementation()
{
	if (BaseCharacter->GetLocalRole() == ROLE_SimulatedProxy)
	{
		StartSprint();
	}
}

void UXyzBaseCharMovementComponent::StopSprint()
{
	if (!bIsSprinting)
	{
		return;
	}

	bIsSprinting = false;
	bForceMaxAccel = 0;
	BaseCharacter->OnSprintStop();

	if (BaseCharacter->GetLocalRole() == ROLE_Authority)
	{
		Multicast_StopSprint();
	}
}

void UXyzBaseCharMovementComponent::Multicast_StopSprint_Implementation()
{
	if (BaseCharacter->GetLocalRole() == ROLE_SimulatedProxy)
	{
		StopSprint();
	}
}

// Sliding

bool UXyzBaseCharMovementComponent::CanSlide() const
{
	return bIsSprinting && !bIsSliding && !CharacterAttributesComponent->IsOutOfStamina() && UpdatedComponent && !UpdatedComponent->IsSimulatingPhysics();
}

void UXyzBaseCharMovementComponent::StartSlide()
{
	if (!CanSlide() || BaseCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == SlideCapsuleHalfHeight)
	{
		return;
	}

	const float NewStamina = FMath::Clamp(CharacterAttributesComponent->GetCurrentStamina() - StaminaConsumptionPerSlide, 0.f, CharacterAttributesComponent->GetMaxStamina());
	CharacterAttributesComponent->SetCurrentStamina(NewStamina);

	const float ComponentScale = BaseCharacter->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = BaseCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float OldUnscaledRadius = BaseCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	const float ClampedHalfHeight = FMath::Max3(0.f, OldUnscaledRadius, SlideCapsuleHalfHeight);
	BaseCharacter->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, ClampedHalfHeight);

	const float HalfHeightAdjust = OldUnscaledHalfHeight - ClampedHalfHeight;
	const float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	UpdatedComponent->MoveComponent(FVector(0.f, 0.f, -ScaledHalfHeightAdjust), UpdatedComponent->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);

	bIsSliding = true;
	bForceNextFloorCheck = true;
	BaseCharacter->OnStartSlide(HalfHeightAdjust, ScaledHalfHeightAdjust);

	if (BaseCharacter->GetLocalRole() == ROLE_Authority)
	{
		Multicast_StartSlide();
	}
}

void UXyzBaseCharMovementComponent::Multicast_StartSlide_Implementation()
{
	if (BaseCharacter->GetLocalRole() == ROLE_SimulatedProxy)
	{
		StartSlide();
	}
}

void UXyzBaseCharMovementComponent::StopSlide()
{
	if (!HasValidData())
	{
		return;
	}

	ACharacter* DefaultCharacter = BaseCharacter->GetClass()->GetDefaultObject<ACharacter>();

	if (BaseCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight())
	{
		bIsSliding = false;
		BaseCharacter->OnStopSlide(0.f, 0.f);
		return;
	}

	const float CurrentHalfHeight = BaseCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float ComponentScale = BaseCharacter->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = BaseCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float HalfHeightAdjust = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - OldUnscaledHalfHeight;
	const float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	const FVector PawnLocation = UpdatedComponent->GetComponentLocation();

	check(BaseCharacter->GetCapsuleComponent());
	const UWorld* MyWorld = GetWorld();
	const float SweepInflation = KINDA_SMALL_NUMBER * 10.f;
	FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(ProneTrace), false, BaseCharacter);
	FCollisionResponseParams ResponseParam;
	InitCollisionParams(CapsuleParams, ResponseParam);
	const FCollisionShape StandingCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -SweepInflation - ScaledHalfHeightAdjust);
	const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
	bool bEncroached = true;

	FVector StandingLocation = PawnLocation + (StandingCapsuleShape.GetCapsuleHalfHeight() - CurrentHalfHeight) * FVector::UpVector;
	bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);

	if (bEncroached) {

		if (IsMovingOnGround())
		{
			const float MinFloorDist = KINDA_SMALL_NUMBER * 10.f;
			if (CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
			{
				StandingLocation.Z -= CurrentFloor.FloorDist - MinFloorDist;
				bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
			}
		}
	}

	BaseCharacter->OnStopSlide(HalfHeightAdjust, ScaledHalfHeightAdjust);
	UpdatedComponent->MoveComponent(StandingLocation - PawnLocation, UpdatedComponent->GetComponentQuat(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
	bForceNextFloorCheck = true;
	bIsSliding = false;
	BaseCharacter->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), true);
	AdjustProxyCapsuleSize();

	if (bEncroached)
	{
		Crouch();
	}
}

void UXyzBaseCharMovementComponent::UnCrouch(const bool bClientSimulation/* = false*/)
{
	if (!BaseCharacter->CanUnCrouch())
	{
		return;
	}

	Super::UnCrouch(bClientSimulation);
}

// Proning

bool UXyzBaseCharMovementComponent::CanProneInCurrentState() const
{
	return (IsFalling() || IsMovingOnGround()) && UpdatedComponent && !UpdatedComponent->IsSimulatingPhysics();
}

void UXyzBaseCharMovementComponent::Prone()
{
	if (!CanProneInCurrentState() || BaseCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == ProneCapsuleHalfHeight)
	{
		return;
	}

	const float ComponentScale = BaseCharacter->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = BaseCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float OldUnscaledRadius = ProneCapsuleRadius;
	const float ClampedProneHalfHeight = FMath::Max3(0.f, OldUnscaledRadius, ProneCapsuleHalfHeight);
	BaseCharacter->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, ClampedProneHalfHeight);
	const float HalfHeightAdjust = OldUnscaledHalfHeight - ClampedProneHalfHeight;
	const float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	UpdatedComponent->MoveComponent(FVector(0.f, 0.f, -ScaledHalfHeightAdjust), UpdatedComponent->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);

	bIsProne = true;

	bForceNextFloorCheck = true;

	BaseCharacter->OnStartProne(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void UXyzBaseCharMovementComponent::UnProne()
{
	if (!HasValidData() || !BaseCharacter->CanUnProne())
	{
		return;
	}

	ACharacter* DefaultCharacter = BaseCharacter->GetClass()->GetDefaultObject<ACharacter>();

	const float OldUnscaledRadius = BaseCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	const float ClampedCrouchHalfHeight = FMath::Max3(0.f, OldUnscaledRadius, CrouchedHalfHeight);

	if (BaseCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == ClampedCrouchHalfHeight)
	{
		bIsProne = false;
		BaseCharacter->OnEndProne(0.f, 0.f);
		return;
	}

	const float CurrentProneHalfHeight = BaseCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const float ComponentScale = BaseCharacter->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = BaseCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float HalfHeightAdjust = ClampedCrouchHalfHeight - OldUnscaledHalfHeight;
	const float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	const FVector PawnLocation = UpdatedComponent->GetComponentLocation();

	check(BaseCharacter->GetCapsuleComponent());

	const UWorld* MyWorld = GetWorld();
	const float SweepInflation = KINDA_SMALL_NUMBER * 10.f;
	FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(ProneTrace), false, BaseCharacter);
	FCollisionResponseParams ResponseParam;
	InitCollisionParams(CapsuleParams, ResponseParam);

	const FCollisionShape CrouchingCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -SweepInflation - ScaledHalfHeightAdjust);
	const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
	bool bEncroached = true;

	FVector CrouchingLocation = PawnLocation + (CrouchingCapsuleShape.GetCapsuleHalfHeight() - CurrentProneHalfHeight) * FVector::UpVector;
	bEncroached = MyWorld->OverlapBlockingTestByChannel(CrouchingLocation, FQuat::Identity, CollisionChannel, CrouchingCapsuleShape, CapsuleParams, ResponseParam);

	if (bEncroached)
	{
		if (IsMovingOnGround())
		{
			const float MinFloorDist = KINDA_SMALL_NUMBER * 10.f;
			if (CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
			{
				CrouchingLocation.Z -= CurrentFloor.FloorDist - MinFloorDist;
				bEncroached = MyWorld->OverlapBlockingTestByChannel(CrouchingLocation, FQuat::Identity, CollisionChannel, CrouchingCapsuleShape, CapsuleParams, ResponseParam);
			}
		}
	}

	if (bEncroached)
	{
		return;
	}

	UpdatedComponent->MoveComponent(CrouchingLocation - PawnLocation, UpdatedComponent->GetComponentQuat(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
	bForceNextFloorCheck = true;
	bIsProne = false;

	BaseCharacter->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), ClampedCrouchHalfHeight, true);
	AdjustProxyCapsuleSize();
	BaseCharacter->OnEndProne(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

// Mantling

bool UXyzBaseCharMovementComponent::IsMantling() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Mantling && UpdatedComponent;
}

void UXyzBaseCharMovementComponent::StartMantle(const FMantlingMovementParameters& MantlingParameters)
{
	BaseCharacter->SetActorEnableCollision(false);
	CurrentMantlingParameters = MantlingParameters;
	OffsetFromMantlingTarget = CurrentMantlingParameters.TargetLocation - CurrentMantlingParameters.TargetActor->GetComponentLocation();
	SetMovementMode(EMovementMode::MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Mantling);
}

void UXyzBaseCharMovementComponent::EndMantle()
{
	BaseCharacter->SetActorEnableCollision(true);
	SetMovementMode(MOVE_Walking);
}

void UXyzBaseCharMovementComponent::PhysMantling(float DeltaTime, int32 Iterations)
{
	const float ElapsedTime = GetWorld()->GetTimerManager().GetTimerElapsed(MantlingTimer) + CurrentMantlingParameters.StartTime;
	const FVector MantlingCurveValue = CurrentMantlingParameters.MantlingCurve->GetVectorValue(ElapsedTime);
	const float PositionAlpha = MantlingCurveValue.X;
	const float XYCorrectionAlpha = MantlingCurveValue.Y;
	const float ZCorrectionAlpha = MantlingCurveValue.Z;
	FVector CorrectedInitialLocation = FMath::Lerp(CurrentMantlingParameters.InitialLocation, CurrentMantlingParameters.InitialAnimationLocation, XYCorrectionAlpha);
	CorrectedInitialLocation.Z = FMath::Lerp(CurrentMantlingParameters.InitialLocation.Z, CurrentMantlingParameters.InitialAnimationLocation.Z, ZCorrectionAlpha);
	const FVector CurrentTargetActorLocation = CurrentMantlingParameters.TargetActor->GetComponentLocation();
	const FVector NewLocation = FMath::Lerp(CorrectedInitialLocation, CurrentTargetActorLocation + OffsetFromMantlingTarget, PositionAlpha);
	const FRotator NewRotation = FMath::Lerp(CurrentMantlingParameters.InitialRotation, CurrentMantlingParameters.TargetRotation, PositionAlpha);
	const FVector Delta = NewLocation - GetActorLocation();
	FHitResult Hit;

	Velocity = Delta / DeltaTime;
	SafeMoveUpdatedComponent(Delta, NewRotation, false, Hit);
}

// Interactive Actors / Ladder

bool UXyzBaseCharMovementComponent::IsOnLadder() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Ladder && UpdatedComponent;
}

bool UXyzBaseCharMovementComponent::IsOnTopOfCurrentLadder() const
{
	if (CurrentLadder.IsValid())
	{
		return CurrentLadder->IsOnTop();
	}
	return false;
}

float UXyzBaseCharMovementComponent::GetCharacterProjectionToCurrentLadder(const FVector Location) const
{
	if (CurrentLadder.IsValid())
	{
		const FVector LadderToCharacterVector = Location - CurrentLadder->GetActorLocation();
		return FVector::DotProduct(CurrentLadder->GetActorUpVector(), LadderToCharacterVector);
	}

	return 0.f;
}

void UXyzBaseCharMovementComponent::AttachCharacterToLadder(ALadder* Ladder)
{
	CurrentLadder = Ladder;
	const FVector DepthOffset = CurrentLadder.Get()->GetActorForwardVector() * CharacterOffsetFromLadder;
	const FVector ProjectionToLadder = GetCharacterProjectionToCurrentLadder(GetActorLocation()) * CurrentLadder->GetActorUpVector();
	FVector StartLocation = CurrentLadder->GetActorLocation() + ProjectionToLadder + DepthOffset;
	FRotator StartRotation = CurrentLadder->GetActorForwardVector().ToOrientationRotator();
	StartRotation.Yaw += 180.f;

	if (CurrentLadder->IsOnTop())
	{
		StartLocation = CurrentLadder->GetAttachFromTopAnimMontageStartLocation();
	}

	BaseCharacter->SetActorLocation(StartLocation);
	BaseCharacter->SetActorRotation(StartRotation);

	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Ladder);
}

void UXyzBaseCharMovementComponent::DetachCharacterFromLadder(const EDetachFromLadderMethod DetachFromLadderMethod /*= EDetachFromLadderMethod::Fall*/)
{
	switch (DetachFromLadderMethod)
	{
	case EDetachFromLadderMethod::JumpOff:
	{
		const FVector JumpDirection = CurrentLadder->GetActorForwardVector();
		SetMovementMode(MOVE_Falling);

		const FVector JumpVelocity = JumpDirection * JumpOffLadderSpeed;

		ForceTargetRotation = JumpDirection.ToOrientationRotator();
		bForceRotation = true;

		Launch(JumpVelocity);
		break;
	}
	case EDetachFromLadderMethod::ReachingTheTop:
	{
		BaseCharacter->Mantle(true);
		break;
	}
	case EDetachFromLadderMethod::ReachingTheBottom:
	{
		SetMovementMode(MOVE_Walking);
		break;
	}
	case EDetachFromLadderMethod::Fall:
	default:
	{
		SetMovementMode(MOVE_Falling);
		break;
	}
	}
}

float UXyzBaseCharMovementComponent::GetLadderSpeedRatio() const
{
	if (CurrentLadder.IsValid())
	{
		return FVector::DotProduct(CurrentLadder->GetActorUpVector(), Velocity) / ClimbingOnLadderMaxSpeed;
	}
	return 1.f;
}

void UXyzBaseCharMovementComponent::PhysLadder(const float DeltaTime, int32 Iterations)
{
	CalcVelocity(DeltaTime, 1.f, false, ClimbingOnLadderBrakingDeceleration);
	const FVector Delta = Velocity * DeltaTime;

	if (HasAnimRootMotion())
	{
		FHitResult Hit;
		SafeMoveUpdatedComponent(Delta, BaseCharacter->GetActorRotation(), false, Hit);
		return;
	}

	const FVector NextPosition = GetActorLocation() + Delta;
	const float NextPositionProjection = GetCharacterProjectionToCurrentLadder(NextPosition);

	if (NextPositionProjection < DetachmentDistanceFromBottom)
	{
		DetachCharacterFromLadder(EDetachFromLadderMethod::ReachingTheBottom);
		return;
	}
	if (NextPositionProjection > (CurrentLadder->GetLadderHeight() - DetachmentDistanceFromTop))
	{
		DetachCharacterFromLadder(EDetachFromLadderMethod::ReachingTheTop);
		return;
	}

	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, BaseCharacter->GetActorRotation(), true, Hit);
}

// Interactive Actors / Zipline

bool UXyzBaseCharMovementComponent::IsOnZipline() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Zipline && UpdatedComponent;
}

bool UXyzBaseCharMovementComponent::IsWallRunning() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_WallRun && UpdatedComponent;
}

float UXyzBaseCharMovementComponent::GetCharacterProjectionToCurrentZipline(const FVector Location) const
{
	if (CurrentZipline.IsValid())
	{
		const FVector ZiplineToCharacterVector = Location - CurrentZipline->GetZiplineStartLocation();
		return FVector::DotProduct(CurrentZipline->GetZiplineSpanVector().GetSafeNormal(), ZiplineToCharacterVector);

	}

	return 0.f;
}

void UXyzBaseCharMovementComponent::AttachCharacterToZipline(AZipline* Zipline)
{
	CurrentZipline = Zipline;
	const FRotator StartRotation = CurrentZipline->GetZiplineSpanVector().ToOrientationRotator();
	const FVector StartLocation = CurrentZipline->GetZiplineStartLocation() + StartRotation.RotateVector(ZiplineAttachmentOffset);

	if (BaseCharacter->GetActorLocation().Z < StartLocation.Z)
	{
		return;
	}

	BaseCharacter->SetActorLocation(StartLocation);
	BaseCharacter->SetActorRotation(StartRotation);

	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Zipline);
}

void UXyzBaseCharMovementComponent::DetachCharacterFromZipline(const EDetachFromZiplineMethod DetachFromZiplineMethod)
{
	switch (DetachFromZiplineMethod)
	{
	case EDetachFromZiplineMethod::ReachingTheEnd:
	{
		const FVector ZiplineDirection = CurrentZipline->GetZiplineSpanVector();
		const FVector JumpDirection = FVector(ZiplineDirection.X, ZiplineDirection.Y, 0.f).RotateAngleAxis(JumpOffZiplineAngle, FVector::UpVector).GetSafeNormal();
		SetMovementMode(MOVE_Falling);

		const FVector JumpVelocity = JumpDirection * JumpOffZiplineSpeed;

		ForceTargetRotation = JumpDirection.ToOrientationRotator();
		bForceRotation = true;

		Launch(JumpVelocity);
		break;
	}
	case EDetachFromZiplineMethod::Fall:
	default:
	{
		SetMovementMode(MOVE_Falling);
		break;
	}
	}
}

void UXyzBaseCharMovementComponent::PhysZipline(const float DeltaTime, int32 Iterations)
{
	Velocity = GetMaxSpeed() * CurrentZipline->GetZiplineSpanVector().GetSafeNormal();
	const FVector Delta = Velocity * DeltaTime;

	const FVector NextPosition = GetActorLocation() + Delta;
	const float NextPositionProjection = GetCharacterProjectionToCurrentZipline(NextPosition);

	if (NextPositionProjection > CurrentZipline->GetZiplineLength() - ZiplineDetachmentDistanceFromEnd)
	{
		DetachCharacterFromZipline(EDetachFromZiplineMethod::ReachingTheEnd);
		return;
	}

	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, BaseCharacter->GetActorRotation(), false, Hit);
}

// Wall Running

bool UXyzBaseCharMovementComponent::CanWallRunInCurrentState() const
{
	return IsFalling() && UpdatedComponent && !UpdatedComponent->IsSimulatingPhysics();
}

bool UXyzBaseCharMovementComponent::IsSurfaceWallRunnable(const FVector& SurfaceNormal) const
{
	if (SurfaceNormal.Z > GetWalkableFloorZ() || SurfaceNormal.Z < -KINDA_SMALL_NUMBER)
	{
		return false;
	}

	return true;
}

void UXyzBaseCharMovementComponent::GetWallRunSideAndDirection(const FVector HitNormal, EWallRunSide& OutSide, FVector& OutDirection) const
{
	if (FVector::DotProduct(HitNormal, BaseCharacter->GetActorRightVector()) > 0)
	{
		OutSide = EWallRunSide::Left;
		OutDirection = FVector::CrossProduct(HitNormal, FVector::UpVector).GetSafeNormal();
	}
	else
	{
		OutSide = EWallRunSide::Right;
		OutDirection = FVector::CrossProduct(FVector::UpVector, HitNormal).GetSafeNormal();
	}
}

void UXyzBaseCharMovementComponent::GetUpdatedWallRunDeltaAndRotation(const FHitResult& HitResult, FVector& DisplacementDelta, FRotator& UpdatedCharacterRotation)
{
	const float ElapsedTimeRatio = GetWorld()->GetTimerManager().GetTimerElapsed(WallRunTimer) / WallRunMaxDuration;
	float CurveValue = 0.f;
	if (IsValid(WallRunVerticalDisplacementCurve))
	{
		CurveValue = WallRunVerticalDisplacementCurve->GetFloatValue(ElapsedTimeRatio);
	}
	FVector WallUpVector;
	float UpdatedAngleOffset;
	FVector Axis;
	float WallTilt;

	if (CurrentWallRunSide == EWallRunSide::Left)
	{
		WallUpVector = FVector::CrossProduct(CurrentWallRunDirection, HitResult.ImpactNormal).GetSafeNormal();
		FQuat::FindBetweenVectors(FVector::UpVector, WallUpVector).ToAxisAndAngle(Axis, WallTilt);
		WallTilt = FMath::RadiansToDegrees(WallTilt);
		UpdatedAngleOffset = CharacterAngleOffsetFromWallPlane - WallTilt;
	}
	else
	{
		WallUpVector = FVector::CrossProduct(HitResult.ImpactNormal, CurrentWallRunDirection).GetSafeNormal();
		FQuat::FindBetweenVectors(FVector::UpVector, WallUpVector).ToAxisAndAngle(Axis, WallTilt);
		WallTilt = FMath::RadiansToDegrees(WallTilt);
		UpdatedAngleOffset = -CharacterAngleOffsetFromWallPlane + WallTilt;
	}

	const FVector UpdatedDisplacement = WallRunMaxVerticalDisplacement * CurveValue * WallUpVector;
	DisplacementDelta = UpdatedDisplacement - PreviousVerticalDisplacement;
	PreviousVerticalDisplacement = UpdatedDisplacement;

	UpdatedCharacterRotation = CurrentWallRunDirection.ToOrientationRotator();
	UpdatedCharacterRotation.Roll += UpdatedAngleOffset;
}

void UXyzBaseCharMovementComponent::StartWallRun(const FHitResult& Hit)
{
	if (!CanWallRunInCurrentState() || !IsSurfaceWallRunnable(Hit.ImpactNormal) || !AttachCharacterToRunnableWall(Hit))
	{
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(WallRunTimer, this, &UXyzBaseCharMovementComponent::EndWallRun, WallRunMaxDuration, false);
	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_WallRun);
	BaseCharacter->OnWallRunStart();
}

void UXyzBaseCharMovementComponent::EndWallRun()
{
	if (IsWallRunning())
	{
		SetPlaneConstraintNormal(FVector::ZeroVector);
		DetachCharacterFromRunnableWall(EDetachFromRunnableWallMethod::Fall);
		GetWorld()->GetTimerManager().ClearTimer(WallRunTimer);;
	}
}

bool UXyzBaseCharMovementComponent::AttachCharacterToRunnableWall(const FHitResult& Hit)
{
	EWallRunSide Side = EWallRunSide::None;
	FVector Direction = FVector::ZeroVector;
	GetWallRunSideAndDirection(Hit.ImpactNormal, Side, Direction);
	if (Side == CurrentWallRunSide && !bCanUseSameWallRunSide)
	{
		return false;
	}

	CurrentWallRunSide = Side;
	CurrentWallRunDirection = Direction;

	FVector UpdatedOffsetFromWallPlane = CharacterOffsetFromWallPlane;
	UpdatedOffsetFromWallPlane.Y = CurrentWallRunSide == EWallRunSide::Left ? UpdatedOffsetFromWallPlane.Y : -UpdatedOffsetFromWallPlane.Y;
	WallRunStartLocation = Hit.ImpactPoint + CurrentWallRunDirection.ToOrientationRotator().RotateVector(UpdatedOffsetFromWallPlane);
	BaseCharacter->SetActorLocation(WallRunStartLocation);

	PreviousVerticalDisplacement = FVector::ZeroVector;
	SetPlaneConstraintNormal(FVector::UpVector);

	return true;
}

void UXyzBaseCharMovementComponent::DetachCharacterFromRunnableWall(const EDetachFromRunnableWallMethod DetachFromRunnableWallMethod /*= EDetachFromRunnableWallMethod::Fall*/)
{
	FRotator NewRotation = BaseCharacter->GetActorRotation();
	NewRotation.Roll = 0.f;
	BaseCharacter->SetActorRotation(NewRotation);

	switch (DetachFromRunnableWallMethod)
	{
	case EDetachFromRunnableWallMethod::JumpOff:
	{
		const FVector WallRunDirectionRightVector = FVector::CrossProduct(FVector::UpVector, CurrentWallRunDirection);
		FVector JumpDirection = CurrentWallRunSide == EWallRunSide::Left ? WallRunDirectionRightVector : -WallRunDirectionRightVector;
		JumpDirection += Velocity.GetSafeNormal();
		SetMovementMode(MOVE_Falling);

		const FVector JumpVelocity = JumpDirection * JumpOffWallRunSpeed;

		ForceTargetRotation = JumpDirection.ToOrientationRotator();
		bForceRotation = true;
		Launch(JumpVelocity);

		bCanUseSameWallRunSide = true;
		break;
	}
	case EDetachFromRunnableWallMethod::Fall:
	default:
	{
		SetMovementMode(MOVE_Falling);
		break;
	}
	}

	BaseCharacter->OnWallRunEnd();
}

bool UXyzBaseCharMovementComponent::UpdateWallRunVelocity(FHitResult& HitResult)
{
	const FVector StartPosition = BaseCharacter->GetActorLocation();
	FVector RightVector = BaseCharacter->GetActorRightVector();
	RightVector.Z = 0.f;
	const FVector LineTraceDirection = CurrentWallRunSide == EWallRunSide::Right ? RightVector.GetSafeNormal() : -RightVector.GetSafeNormal();
	const FVector EndPosition = StartPosition + WallRunLineTraceLength * LineTraceDirection;

	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartPosition, EndPosition, ECC_WallRunning, FCollisionQueryParams::DefaultQueryParam))
	{
		EWallRunSide Side = EWallRunSide::None;
		FVector Direction = FVector::ZeroVector;
		GetWallRunSideAndDirection(HitResult.ImpactNormal, Side, Direction);

		if (Side != CurrentWallRunSide)
		{
			EndWallRun();
			return false;
		}

		CurrentWallRunDirection = Direction;
		Velocity = GetMaxSpeed() * CurrentWallRunDirection;
	}
	else
	{
		EndWallRun();
		return false;
	}
	return true;
}

void UXyzBaseCharMovementComponent::PhysWallRun(const float DeltaTime, int32 Iterations)
{
	FHitResult HitResult;
	if (!UpdateWallRunVelocity(HitResult))
	{
		return;
	}

	const FVector Delta = Velocity * DeltaTime;

	FVector DisplacementDelta;
	FRotator UpdatedCharacterRotation;
	GetUpdatedWallRunDeltaAndRotation(HitResult, DisplacementDelta, UpdatedCharacterRotation);
	UpdatedCharacterRotation = FMath::RInterpTo(GetLastUpdateRotation(), UpdatedCharacterRotation, DeltaTime, WallRunRotationInterpSpeed);

	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta + DisplacementDelta, UpdatedCharacterRotation, false, Hit);
}

void FSavedMove_XyzCharacter::Clear()
{
	Super::Clear();

	bSavedIsSprinting = 0;
	bSavedIsMantling = 0;
	bSavedIsPressingSlide = 0;
}

void FSavedMove_XyzCharacter::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel,
	FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	checkf(Character->IsA<AXyzBaseCharacter>(), TEXT("FSavedMove_XyzCharacter::SetMoveFor: AXyzBaseCharacter should be used."))
		const AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(Character);

	const UXyzBaseCharMovementComponent* BaseCharacterMovementComponent = BaseCharacter->GetBaseCharacterMovementComponent();
	bSavedIsSprinting = BaseCharacterMovementComponent->bIsSprinting;
	bSavedIsMantling = BaseCharacterMovementComponent->IsMantling();
	bSavedIsPressingSlide = BaseCharacterMovementComponent->bIsSliding;
}

bool FSavedMove_XyzCharacter::CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* InCharacter,
	float MaxDelta) const
{
	const FSavedMove_XyzCharacter* NewMove = StaticCast<FSavedMove_XyzCharacter*>(NewMovePtr.Get());

	if (NewMove->bSavedIsSprinting != bSavedIsSprinting || NewMove->bSavedIsMantling != bSavedIsMantling || NewMove->bSavedIsPressingSlide != bSavedIsPressingSlide)
	{
		return false;
	}

	return Super::CanCombineWith(NewMovePtr, InCharacter, MaxDelta);
}

void FSavedMove_XyzCharacter::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	checkf(Character->IsA<AXyzBaseCharacter>(), TEXT("FSavedMove_XyzCharacter::PrepMoveFor: AXyzBaseCharacter should be used."))
		AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(Character);

	UXyzBaseCharMovementComponent* BaseCharacterMovementComponent = BaseCharacter->GetBaseCharacterMovementComponent();

	if (bSavedIsSprinting)
	{
		BaseCharacterMovementComponent->StartSprint();
	}
	else
	{
		BaseCharacterMovementComponent->StopSprint();
	}

	if (bSavedIsPressingSlide)
	{
		BaseCharacterMovementComponent->StartSlide();
	}
}

uint8 FSavedMove_XyzCharacter::GetCompressedFlags() const
{
	//  FLAG_JumpPressed = 0x01,	// Jump pressed
	//	FLAG_WantsToCrouch = 0x02,	// Wants to crouch
	//	FLAG_Reserved_1 = 0x04,	// Reserved for future use
	//	FLAG_Reserved_2 = 0x08,	// Reserved for future use
	//	// Remaining bit masks are available for custom flags.
	//	FLAG_Custom_0 = 0x10, // Sprinting
	//	FLAG_Custom_1 = 0x20, // Mantling
	//	FLAG_Custom_2 = 0x40, // PressingSlide
	//	FLAG_Custom_3 = 0x80,

	uint8 Result = Super::GetCompressedFlags();

	if (bSavedIsSprinting)
	{
		Result |= FLAG_Custom_0;
	}
	if (bSavedIsMantling)
	{
		Result &= ~FLAG_JumpPressed;
		Result |= FLAG_Custom_1;
	}
	if (bSavedIsPressingSlide)
	{
		Result |= FLAG_Custom_2;
	}

	return Result;
}

FNetworkPredictionData_Client_XyzCharacter::FNetworkPredictionData_Client_XyzCharacter(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr FNetworkPredictionData_Client_XyzCharacter::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_XyzCharacter());
}
