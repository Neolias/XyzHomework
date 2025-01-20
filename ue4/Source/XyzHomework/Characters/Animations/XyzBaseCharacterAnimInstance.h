// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Animation/AnimInstance.h"
#include "XyzBaseCharacterAnimInstance.generated.h"

/**
 *
 */
UCLASS()
class XYZHOMEWORK_API UXyzBaseCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animation")
	bool bIsFalling = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animation")
	bool bIsCrouching = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animation")
	bool bIsSprinting = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animation")
	bool bIsOutOfStamina = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animation")
	bool bIsProne = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animation")
	bool bIsSwimming = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animation")
	bool bIsOnLadder = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animation")
	bool bIsOnTopOfCurrentLadder = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animation")
	bool bIsOnZipline = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animation")
	bool bIsWallRunning = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animation")
	bool bIsSliding = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animation")
	float LadderSpeedRatio = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animation")
	bool bIsAiming = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animation")
	bool bIsReloading = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animation")
	bool bIsPrimaryItemEquipped = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animation")
	FVector Velocity = FVector::ZeroVector; 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animation", meta = (UIMin = -180.f, UIMax = 180.f))
	float MovementDirection = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animation", meta = (UIMin = 0.f))
	float MovementSpeed = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animation")
	EWallRunSide CurrentWallRunSide = EWallRunSide::None;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animation")
	EEquipmentItemType CurrentRangedWeaponType = EEquipmentItemType::None;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animation", meta = (UIMin = -180.f, UIMax = 180.f))
	float CharacterPitch = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Camera", meta = (UIMin = -180.f, UIMax = 180.f))
	float CameraPitch = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Camera", meta = (UIMin = -180.f, UIMax = 180.f))
	FRotator AimRotation = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | IK Settings")
	FVector LeftFootEffectorLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | IK Settings")
	FVector RightFootEffectorLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | IK Settings")
	FVector PelvisEffectorLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | IK Settings")
	FTransform CurrentRangedWeaponForeGripTransform;

	TWeakObjectPtr<class AXyzBaseCharacter> CachedBaseCharacter;

	virtual float CalculateCameraPitch();
};
