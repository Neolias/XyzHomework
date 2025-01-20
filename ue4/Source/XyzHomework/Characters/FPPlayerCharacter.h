// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/PlayerCharacter.h"
#include "Components/TimelineComponent.h"
#include "FPPlayerCharacter.generated.h"

/**
 *
 */
UCLASS()
class XYZHOMEWORK_API AFPPlayerCharacter : public APlayerCharacter
{
	GENERATED_BODY()

public:
	explicit AFPPlayerCharacter(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// General

	virtual FRotator GetViewRotation() const override;

	// Sliding

	virtual void OnStartSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnStopSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	// Crouching

	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	// Proning

	virtual void OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "XYZ Character | First Person")
	USkeletalMeshComponent* FPMeshComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "XYZ Character | First Person | Camera")
	FName CameraSocket = FName("CameraSocket");
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "XYZ Character | First Person | Camera", meta = (ClampMin = 0.f, UIMin = 0.f))
	float AlignmentBlendSpeed = 10.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "XYZ Character | First Person | Camera", meta = (UIMin = -89.9f, UIMax = 89.9f))
	float LadderCameraMinPitch = -60.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "XYZ Character | First Person | Camera", meta = (UIMin = -89.9f, UIMax = 89.9f))
	float LadderCameraMaxPitch = 80.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "XYZ Character | First Person | Camera", meta = (UIMin = -359.9f, UIMax = 0.f))
	float LadderCameraMinYaw = -60.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "XYZ Character | First Person | Camera", meta = (UIMin = 0.f, UIMax = 359.9f))
	float LadderCameraMaxYaw = 60.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "XYZ Character | First Person | Camera", meta = (UIMin = -89.9f, UIMax = 89.9f))
	float ZiplineCameraMinPitch = -89.9f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "XYZ Character | First Person | Camera", meta = (UIMin = -89.9f, UIMax = 89.9f))
	float ZiplineCameraMaxPitch = 89.9f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "XYZ Character | First Person | Camera", meta = (UIMin = -359.9f, UIMax = 0.f))
	float ZiplineCameraMinYaw = -90.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "XYZ Character | First Person | Camera", meta = (UIMin = 0.f, UIMax = 359.9f))
	float ZiplineCameraMaxYaw = 90.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "XYZ Character | First Person | Camera")
	UCurveFloat* WallRunCameraTiltCurve;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "XYZ Character | Movement | Sliding")
	UAnimMontage* SlideFPAnimMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "XYZ Character | Movement | Landing")
	UAnimMontage* HardLandFPAnimMontage;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "XYZ Character | First Person | Camera")
	UCameraComponent* FPCameraComponent;

	// General
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

	// Landing

	virtual void OnHardLandStart() override;
	virtual void OnHardLandEnd() override;

	// OutOfStamina

	virtual void OnOutOfStaminaStart() override;
	virtual void OnOutOfStaminaEnd() override;

	// Mantling

	virtual void OnMantle(const FMantlingSettings& MantlingSettings, const FMantlingMovementParameters& MantlingParameters) override;

	// Interactive Actors

	virtual void OnAttachedToLadderFromTop(ALadder* Ladder) override;

	// Death

	virtual void OnDeathStarted() override;

private:
	FTimerHandle FPMantlingTimer;
	FTimerHandle FPLadderTimer;
	FTimerHandle FPSlideTimer;
	FTimeline WallRunCameraTiltTimeline;
	bool bIsForcedToAlignFPCamera = false;
	bool bWantsToEndMantle = false;
	bool bWantsToAttachToLadder = false;
	bool bWantsToEndOutOfStamina = false;
	bool bWantsToEndHardLand = false;
	float AnimMontageCameraBlendSpeed = 50.f;
	FRotator ForcedTargetControlRotation = FRotator::ZeroRotator;

	// General

	bool IsFPMontagePlaying() const;

	// Camera

	bool IsAligningFPCameraToSocketRotation() const;
	void StartFPCameraAlignment();
	void EndFPCameraAlignment();
	void UpdateCameraAlignment(float DeltaSeconds);

	// Mantling

	void OnEndMantle();

	// Interactive Actors

	void OnAttachedToLadder();
	void OnDetachedFromLadder();
	void OnAttachedToZipline();
	void OnDetachedFromZipline();

	// Wall Running

	UFUNCTION()
	void UpdateWallRunCameraTilt(float Value) const;
	void StartWallRunCameraTilt() { WallRunCameraTiltTimeline.Play(); }
	void EndWallRunCameraTilt() { WallRunCameraTiltTimeline.Reverse(); }
};
