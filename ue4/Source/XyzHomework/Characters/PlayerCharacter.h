// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/TimelineComponent.h"
#include "PlayerCharacter.generated.h"

/**
 *
 */
UCLASS(Blueprintable)
class XYZHOMEWORK_API APlayerCharacter : public AXyzBaseCharacter
{
	GENERATED_BODY()

public:
	explicit APlayerCharacter(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

	// General Movement

	virtual void MoveForward(float Value) override;
	virtual void MoveRight(float Value) override;
	virtual void Turn(float Value) override;
	virtual void LookUp(float Value) override;
	virtual void TurnAtRate(float Value) override;
	virtual void LookUpAtRate(float Value) override;

	// Jumping
	virtual void Jump() override;
	virtual bool CanJumpInternal_Implementation() const override;
	virtual void OnJumped_Implementation() override;

	// Swimming

	virtual void SwimForward(float Value) override;
	virtual void SwimRight(float Value) override;
	virtual void SwimUp(float Value) override;
	virtual void Dive() override;

	// Sliding

	virtual void OnStartSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnStopSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	// Crouching / Proning

	virtual bool CanUnCrouch() override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual bool CanUnProne() override;
	virtual void OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	// Interactive Actors

	virtual void ClimbLadderUp(float Value) override;

	// Wall Running

	virtual void OnWallRunStart() override;
	virtual void OnWallRunEnd() override;


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "XYZ Character | Camera")
	class UCameraComponent* CameraComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "XYZ Character | Camera")
	class USpringArmComponent* SpringArmComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "XYZ Character | Camera")
	float ProneCameraHeightOffset = -50.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "XYZ Character | Camera")
	float ProneCameraProximityOffset = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "XYZ Character | Camera")
	float ProneCameraRightOffset = 50.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "XYZ Character | Camera")
	UCurveFloat* ProneCameraTimelineCurve;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "XYZ Character | Camera")
	UCurveFloat* AimingFOVTimelineCurve;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "XYZ Character | Camera")
	UCurveFloat* WallRunCameraTimelineCurve;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "XYZ Character | Camera", meta = (ClampMin = 0.f, ClampMax = 1.f, UIMin = 0.f, UIMax = 1.f))
	float AimTurnModifier = 0.75f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "XYZ Character | Camera", meta = (ClampMin = 0.f, ClampMax = 1.f, UIMin = 0.f, UIMax = 1.f))
	float AimLookUpModifier = 0.75f;

	TWeakObjectPtr<APlayerCameraManager> CachedCameraManager;
	float CameraHeight = 0.f;
	FVector CachedLastVelocity = FVector(KINDA_SMALL_NUMBER, KINDA_SMALL_NUMBER, 1.f);
	FVector CachedSpringArmSocketOffset = FVector::ZeroVector;
	FVector CachedSpringArmTargetOffset = FVector::ZeroVector;
	FVector NewSpringArmSocketOffsetDelta = FVector::ZeroVector;
	FVector NewSpringArmTargetOffsetDelta = FVector::ZeroVector;
	FTimeline AimingFOVTimeline;
	FTimeline WallRunCameraTimeline;
	FTimeline ProneCameraTimeline;
	bool bShouldSkipProneTimeline = false;
	float DefaultCameraFOV = 0.f;
	float TargetAimingFOV = 0.f;

	bool IsCameraManagerValid();

	// Aiming

	virtual void OnStartAimingInternal() override;
	virtual void OnStopAimingInternal() override;
	float GetAimTurnModifier() const;
	float GetAimLookUpModifier() const;

	// Timelines

	UFUNCTION()
	virtual void UpdateProneCameraTimeline(float Value);
	UFUNCTION()
	virtual void UpdateAimingFOVTimeline(float Value);
	UFUNCTION()
	virtual void UpdateWallRunCameraTimeline(float Value);
};
