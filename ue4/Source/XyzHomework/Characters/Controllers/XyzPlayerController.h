// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "XyzPlayerController.generated.h"

class UPlayerHUDWidget;
/**
 *
 */
UCLASS()
class XYZHOMEWORK_API AXyzPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void SetPawn(APawn* InPawn) override;
	bool IgnoresFPCameraPitch() const;
	void ShouldIgnoreFPCameraPitch(const bool bIgnoreFPCameraPitch_In)
	{
		bIgnoresFPCameraPitch = bIgnoreFPCameraPitch_In;
	}

protected:
	virtual void SetupInputComponent() override;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UPlayerHUDWidget> PlayerHUDWidgetClass;

private:
	TWeakObjectPtr<class AXyzBaseCharacter> CachedBaseCharacter;
	bool bIgnoresFPCameraPitch = false;
	UPROPERTY()
	UPlayerHUDWidget* PlayerHUDWidget;

	void CreateAndInitializeHUDWidgets();

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void ChangeCrouchState();
	void Jump();
	void StartSprint();
	void StopSprint();
	void ChangeProneState();
	void Mantle();
	void SwimForward(float Value);
	void SwimRight(float Value);
	void SwimUp(float Value);
	void Dive();
	void ClimbLadderUp(float Value);
	void InteractWithLadder();
	void InteractWithZipline();
	void JumpOffRunnableWall();
	void Slide();
	void ReloadLevel();
	void StartWeaponFire();
	void StopFire();
	void StartAim();
	void EndAim();
	void ReloadWeapon();
	void DrawNextEquipmentItem();
	void DrawPreviousEquipmentItem();
	void TogglePrimaryItem();
	void ThrowItem();
	void ActivateNextWeaponMode();
	void UsePrimaryMeleeAttack();
	void UseSecondaryMeleeAttack();

	void TurnAtRate(float Value);
	void LookUpAtRate(float Value);
};
