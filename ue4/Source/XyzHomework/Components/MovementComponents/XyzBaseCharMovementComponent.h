// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "XyzGenericStructs.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "XyzBaseCharMovementComponent.generated.h"

class ALadder;
class AZipline;

/**
 *
 */
UCLASS()
class XYZHOMEWORK_API UXyzBaseCharMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	friend class FSavedMove_XyzCharacter;

public:
	virtual void BeginPlay() override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	bool IsSprinting() const { return bIsSprinting; }
	bool IsProne() const { return bIsProne; }
	bool GetWantsToProne() const { return bWantsToProne; }
	void SetWantsToProne(const bool bWantsToProne_In)
	{
		bWantsToProne = bWantsToProne_In;
	}
	bool IsSliding() const { return bIsSliding; }
	float GetProneCapsuleHalfHeight() const { return ProneCapsuleHalfHeight; }
	ALadder* GetCurrentLadder() const { return CurrentLadder.Get(); }
	AZipline* GetCurrentZipline() const { return CurrentZipline.Get(); }
	EWallRunSide GetCurrentWallRunSide() const { return CurrentWallRunSide; }
	FVector GetCurrentWallRunDirection() const { return CurrentWallRunDirection; }

	// Swimming

	bool IsSwimmingUnderWater(FVector Delta = FVector::ZeroVector) const;
	bool IsSwimmingOnWaterPlane() const;
	bool IsDiving() const { return bIsDiving; }
	void SwimUp(float Value);
	void StartDive();

	// Sprinting / Sliding

	void StartSprint();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartSprint();
	void StopSprint();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopSprint();
	bool CanSlide() const;
	void StartSlide();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartSlide();
	void StopSlide();

	// Crouching

	virtual void UnCrouch(bool bClientSimulation = false) override;

	// Proning

	void Prone();
	void UnProne();

	// Mantling

	bool IsMantling() const;
	void StartMantle(const FMantlingMovementParameters& MantlingParameters);

	// Interactive Actors

	bool IsOnLadder() const;
	bool IsOnTopOfCurrentLadder() const;
	void AttachCharacterToLadder(ALadder* Ladder);
	void DetachCharacterFromLadder(EDetachFromLadderMethod DetachFromLadderMethod);
	float GetLadderSpeedRatio() const;
	bool IsOnZipline() const;
	void AttachCharacterToZipline(AZipline* Zipline);
	void DetachCharacterFromZipline(EDetachFromZiplineMethod DetachFromZiplineMethod);

	// Wall Running

	bool IsWallRunning() const;
	void StartWallRun(const FHitResult& Hit);
	void DetachCharacterFromRunnableWall(EDetachFromRunnableWallMethod DetachFromRunnableWallMethod);

protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual float GetMaxSpeed() const override;
	virtual void PhysicsRotation(float DeltaTime) override;
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
	virtual void PhysSwimming(float DeltaTime, int32 Iterations) override;
	virtual bool CanAttemptJump() const override;
	virtual bool DoJump(bool bReplayingMoves) override;

private:
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Swimming", meta = (ClampMin = 0.f, UIMin = 0.f))
	float SwimmingCapsuleHalfHeight = 34.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Swimming", meta = (ClampMin = 0.f, UIMin = 0.f))
	float SwimmingCapsuleRadius = 34.f;
	UPROPERTY(EditDefaultsOnly, Category = "XYZ Character | Swimming")
	FName HeadBoneName = "head";
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Swimming", meta = (ClampMin = 0.f, UIMin = 0.f))
	float SwimPitchRotationInterpSpeed = 1.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Swimming", meta = (ClampMin = 0.f, UIMin = 0.f))
	float DiveSpeed = 300.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Swimming", meta = (ClampMin = 0.f, UIMin = 0.f))
	float DiveActionLength = .2f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Jumping", meta = (ClampMin = 0.f, UIMin = 0.f))
	float StaminaConsumptionPerJump = 20.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Sprinting", meta = (ClampMin = 0.f, UIMin = 0.f))
	float SprintSpeed = 1200.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Sprinting", meta = (ClampMin = 0.f, UIMin = 0.f))
	float OutOfStaminaSpeed = 100.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Sliding", meta = (ClampMin = 0.f, UIMin = 0.f))
	float SlideSpeed = 1000.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Sliding", meta = (ClampMin = 0.f, UIMin = 0.f))
	float SlideCapsuleHalfHeight = 60.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character Sliding", meta = (ClampMin = 0.f, UIMin = 0.f))
	float StaminaConsumptionPerSlide = 20.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Proning", meta = (ClampMin = 0.f, UIMin = 0.f))
	float ProneSpeed = 50.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Proning", meta = (ClampMin = 0.f, UIMin = 0.f))
	float ProneCapsuleHalfHeight = 34.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Proning", meta = (ClampMin = 0.f, UIMin = 0.f))
	float ProneCapsuleRadius = 34.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Ladder", meta = (ClampMin = 0.f, UIMin = 0.f))
	float ClimbingOnLadderMaxSpeed = 200.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Ladder", meta = (ClampMin = 0.f, UIMin = 0.f))
	float ClimbingOnLadderBrakingDeceleration = 2048.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Ladder")
	float CharacterOffsetFromLadder = 54.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Ladder")
	float DetachmentDistanceFromTop = 45.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Ladder")
	float DetachmentDistanceFromBottom = 90.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Ladder", meta = (ClampMin = 0.f, UIMin = 0.f))
	float JumpOffLadderSpeed = 500.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Zipline", meta = (ClampMin = 0.f, UIMin = 0.f))
	float ZiplineMovementMaxSpeed = 500.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Zipline")
	FVector ZiplineAttachmentOffset = FVector(50.f, 10.f, -123.f);
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Zipline")
	float ZiplineDetachmentDistanceFromEnd = 100.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Zipline", meta = (ClampMin = 0.f, UIMin = 0.f))
	float JumpOffZiplineSpeed = 500.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Zipline", meta = (UIMin = -180.f, UIMax = 180.f))
	float JumpOffZiplineAngle = 30.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Wall Running", meta = (ClampMin = 0.f, UIMin = 0.f))
	float WallRunMaxSpeed = 600.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Wall Running", meta = (ClampMin = 0.f, UIMin = 0.f))
	float WallRunMaxDuration = 2.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Wall Running", meta = (ClampMin = 0.f, UIMin = 0.f))
	float WallRunRotationInterpSpeed = 20.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Wall Running")
	FVector CharacterOffsetFromWallPlane = FVector(0.f, 45.f, 0.f);
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Wall Running", meta = (ClampMin = 0.f, UIMin = 0.f))
	float WallRunLineTraceLength = 200.f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Wall Running", meta = (ClampMin = 0.f, UIMin = 0.f))
	float WallRunMaxVerticalDisplacement = 150.f;
	UPROPERTY(EditDefaultsOnly, Category = "XYZ Character | Wall Running")
	UCurveFloat* WallRunVerticalDisplacementCurve;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Wall Running")
	float CharacterAngleOffsetFromWallPlane = 7.5f;
	UPROPERTY(EditAnywhere, Category = "XYZ Character | Wall Running", meta = (ClampMin = 0.f, UIMin = 0.f))
	float JumpOffWallRunSpeed = 500.f;

	UPROPERTY()
	class AXyzBaseCharacter* BaseCharacter;
	UPROPERTY()
	class UCharacterAttributesComponent* CharacterAttributesComponent;
	UPROPERTY()
	class UCharacterEquipmentComponent* CharacterEquipmentComponent;
	bool bIsSwimmingOnWaterPlane = false;
	bool bIsDiving = false;
	bool bIsSwimmingUp = false;
	float TargetSwimUpSpeed = 0.f;
	bool bIsSprinting = false;
	bool bIsSliding = false;
	bool bIsProne = false;
	bool bWantsToProne = false;
	bool bCanUseSameWallRunSide = false;
	FRotator ForceTargetRotation = FRotator::ZeroRotator;
	bool bForceRotation = false;
	float WaterPlaneDetectionOffset = 5.f;
	FMantlingMovementParameters CurrentMantlingParameters;
	FTimerHandle MantlingTimer;
	FVector OffsetFromMantlingTarget = FVector::ZeroVector;
	TWeakObjectPtr<ALadder> CurrentLadder;
	TWeakObjectPtr<AZipline> CurrentZipline;
	EWallRunSide CurrentWallRunSide = EWallRunSide::None;
	FVector CurrentWallRunDirection = FVector::ZeroVector;
	FVector WallRunStartLocation = FVector::ZeroVector;
	FVector PreviousVerticalDisplacement = FVector::ZeroVector;
	FTimerHandle WallRunTimer;
	FTimerHandle DiveTimer;

	// Swimming
	void StopDive();

	// Proning

	bool CanProneInCurrentState() const;

	// Mantling

	void EndMantle();
	void PhysMantling(float DeltaTime, int32 Iterations);

	// Interactive Actors

	float GetCharacterProjectionToCurrentLadder(FVector Location) const;
	void PhysLadder(float DeltaTime, int32 Iterations);
	float GetCharacterProjectionToCurrentZipline(FVector Location) const;
	void PhysZipline(float DeltaTime, int32 Iterations);

	// Wall Running

	bool CanWallRunInCurrentState() const;
	bool IsSurfaceWallRunnable(const FVector& SurfaceNormal) const;
	void GetWallRunSideAndDirection(FVector HitNormal, OUT EWallRunSide& OutSide, OUT FVector& OutDirection) const;
	void GetUpdatedWallRunDeltaAndRotation(const FHitResult& HitResult, OUT FVector& DisplacementDelta, OUT FRotator& UpdatedCharacterRotation);
	bool AttachCharacterToRunnableWall(const FHitResult& Hit);
	void EndWallRun();
	bool UpdateWallRunVelocity(FHitResult& HitResult);
	void PhysWallRun(float DeltaTime, int32 Iterations);
};

class FSavedMove_XyzCharacter : public FSavedMove_Character
{
	typedef FSavedMove_Character Super;

public:
	virtual void Clear() override;
	virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
	virtual bool CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* InCharacter, float MaxDelta) const override;
	virtual void PrepMoveFor(ACharacter* Character) override;
	virtual uint8 GetCompressedFlags() const override;

	uint8 bSavedIsSprinting : 1;
	uint8 bSavedIsMantling : 1;
	uint8 bSavedIsPressingSlide : 1;
};

class FNetworkPredictionData_Client_XyzCharacter : public FNetworkPredictionData_Client_Character
{
	typedef FNetworkPredictionData_Client_Character Super;
public:
	FNetworkPredictionData_Client_XyzCharacter(const UCharacterMovementComponent& ClientMovement);

	virtual FSavedMovePtr AllocateNewMove() override;
};
