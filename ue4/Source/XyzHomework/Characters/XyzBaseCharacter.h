// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "XyzGenericStructs.h"
#include "GameFramework/Character.h"
#include "XyzBaseCharacter.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAimingStateChanged, bool)

class UCharacterEquipmentComponent;
class UGCBaseCharacterMovementComponent;
class UCharacterAttributesComponent;
class AInteractiveActor;
typedef TArray<AInteractiveActor*, TInlineAllocator<10>> TInteractiveActorsArray;
class AZipline;
class ALadder;
class AEquipmentItem;

/**
 *
 */
UCLASS(Abstract, NotBlueprintable)
class XYZHOMEWORK_API AXyzBaseCharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	FOnAimingStateChanged OnAimingStateChanged;
	bool bIsSprintRequested = false;
	explicit AXyzBaseCharacter(const OUT FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	class UXyzBaseCharMovementComponent* GetBaseCharacterMovementComponent() const { return BaseCharacterMovementComponent; }
	UCharacterAttributesComponent* GetCharacterAttributesComponent() const { return CharacterAttributesComponent; }
	UCharacterEquipmentComponent* GetCharacterEquipmentComponent() const { return CharacterEquipmentComponent; }
	virtual bool IsFirstPerson() const { return bIsFirstPerson; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetIKLeftFootOffset() const { return IKLeftFootOffset; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetIKRightFootOffset() const { return IKRightFootOffset; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetIKPelvisOffset() const { return IKPelvisOffset; }

	virtual FRotator GetAimOffset();

	// Overrides

	virtual void PossessedBy(AController* NewController) override;
	virtual void Jump() override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

	// General Movement

	virtual void MoveForward(float Value) {};
	virtual void MoveRight(float Value) {};
	virtual void Turn(float Value) {};
	virtual void LookUp(float Value) {};
	virtual void TurnAtRate(float Value) {};
	virtual void LookUpAtRate(float Value) {};

	// Aiming

	bool IsAiming() const { return bIsAiming; }
	void SetIsAiming(const bool bIsAiming_In) { bIsAiming = bIsAiming_In; }
	float GetCurrentAimingMovementSpeed() const { return CurrentAimingMovementSpeed; }
	virtual bool CanAim();
	virtual void SetWantsToAim();
	virtual void ResetWantsToAim();
	UFUNCTION(BlueprintNativeEvent, Category = "XYZ Character | Shooting")
	void OnStartAiming();
	UFUNCTION(BlueprintNativeEvent, Category = "XYZ Character | Shooting")
	void OnStopAiming();

	// Shooting

	virtual bool CanFireWeapon();
	void TryToggleWeaponFire();
	virtual void StartWeaponFire();
	virtual void StopWeaponFire();
	float GetCurrentReloadingWalkSpeed() const { return CurrentReloadingWalkSpeed; }
	virtual void ReloadWeapon();
	virtual void OnWeaponReloaded();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartAutoReload();

	// Melee weapons

	virtual void UsePrimaryMeleeAttack();
	virtual void UseSecondaryMeleeAttack();

	// Equipment Items

	float GetCurrentThrowItemMovementSpeed() const { return CurrentThrowItemMovementSpeed; }
	virtual bool CanSwitchEquipmentItem();
	virtual void DrawNextEquipmentItem();
	virtual void DrawPreviousEquipmentItem();
	virtual bool CanEquipPrimaryItem();
	virtual void TogglePrimaryItem();
	virtual bool CanThrowItem();
	virtual void ThrowItem();
	virtual void ActivateNextWeaponMode();

	// Swimming

	virtual void SwimForward(float Value) {};
	virtual void SwimRight(float Value) {};
	virtual void SwimUp(float Value) {};
	virtual void Dive() {};

	// Sprinting / Sliding

	virtual bool CanSprint();
	virtual void StartSprint();
	virtual void StopSprint();
	UFUNCTION(BlueprintNativeEvent, Category = "XYZ Character | Movement")
	void OnSprintStart();
	UFUNCTION(BlueprintNativeEvent, Category = "XYZ Character | Movement")
	void OnSprintStop();
	virtual void StartSlide();
	virtual void StopSlide();
	virtual void OnStartSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust);
	virtual void OnStopSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust);

	// Crouching / Proning

	virtual bool CanUnCrouch() { return true; }
	virtual void ChangeCrouchState();
	virtual bool CanUnProne() { return true; }
	virtual void ChangeProneState() const;
	virtual void OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust);
	virtual void OnEndProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust);

	// Mantling

	UFUNCTION(BlueprintCallable)
	virtual void Mantle(bool bForceMantle = false);
	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_Mantle(bool bForceMantle = false);

	// Interactive Actors

	virtual void InteractWithZipline();
	virtual void InteractWithLadder();
	virtual void RegisterInteractiveActor(AInteractiveActor* InteractiveActor);
	virtual void UnRegisterInteractiveActor(AInteractiveActor* InteractiveActor);
	virtual void ClimbLadderUp(float Value) {};

	// Wall Running

	virtual void OnWallRunStart() {};
	virtual void OnWallRunEnd() {};
	virtual void JumpOffRunnableWall();

	// Death

	virtual void EnableRagdoll();

	// IGenericTeamAgentInterface
	virtual FGenericTeamId GetGenericTeamId() const override;
	// ~IGenericTeamAgentInterface

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "XYZ Character | Components")
	UCharacterAttributesComponent* CharacterAttributesComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "XYZ Character | Components")
	class UCharacterEquipmentComponent* CharacterEquipmentComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "XYZ Character | Team")
	ETeam Team = ETeam::Enemy;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "XYZ Character | General", meta = (ClampMin = 0.f, UIMin = 0.f))
	float CollisionCapsuleHalfHeight = 90.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "XYZ Character | Controls", meta = (ClampMin = 0.f, UIMin = 0.f))
	float BaseTurnRate = 45.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "XYZ Character | Controls", meta = (ClampMin = 0.f, UIMin = 0.f))
	float BaseLookUpRate = 45.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "XYZ Character | Movement | Landing", meta = (ClampMin = 0.f, UIMin = 0.f))
	float HardLandMinHeight = 500.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "XYZ Character | Movement | Landing")
	UAnimMontage* HardLandAnimMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "XYZ Character | Movement | Sliding")
	UAnimMontage* SlideAnimMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "XYZ Character | Movement | Mantling")
	float MantlingDepth = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "XYZ Character | Movement | Mantling", meta = (ClampMin = 0.f, UIMin = 0.f))
	float ForwardTraceDistance = 65.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "XYZ Character | Movement | Mantling")
	FMantlingSettings HighMantleSettings;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "XYZ Character | Movement | Mantling")
	FMantlingSettings LowMantleSettings;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "XYZ Character | Death")
	UAnimMontage* DeathAnimMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "XYZ Character | IK Settings")
	FName LeftFootSocketName = "foot_l_socket";
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "XYZ Character | IK Settings")
	FName RightFootSocketName = "foot_r_socket";
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "XYZ Character | IK Settings", meta = (ClampMin = 0.f, UIMin = 0.f))
	float IKTraceDistance = 50.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "XYZ Character | IK Settings", meta = (ClampMin = 0.f, UIMin = 0.f))
	float IKInterpSpeed = 15.f;

	TWeakObjectPtr<class AXyzPlayerController> XyzPlayerController;
	UPROPERTY()
	UXyzBaseCharMovementComponent* BaseCharacterMovementComponent;
	UPROPERTY()
	USkeletalMeshComponent* SkeletalMeshComponent;
	TWeakObjectPtr<class ARangedWeaponItem> CurrentRangedWeapon;
	TInteractiveActorsArray InteractiveActors;
	FTimerHandle HardLandTimer;
	bool bIsHardLanding = false;
	bool bIsFirstPerson = false;
	bool bWantsToAim = false;
	bool bIsAiming = false;
	bool bWantsToFire = false;
	float CurrentReloadingWalkSpeed = 0.f;
	float CurrentAimingMovementSpeed = 0.f;
	float CurrentThrowItemMovementSpeed = 0.f;
	float CurrentJumpApexHeight = 0.f;
	float ProneEyeHeight = 0.f;
	float CachedCollisionCapsuleScaledRadius = 0.f;
	float CachedCollisionCapsuleScaledHalfHeight = 0.f;
	float IKLeftFootOffset = 0.f;
	float IKRightFootOffset = 0.f;
	float IKPelvisOffset = 0.f;
	float IKScale = 0.f;

	// Overrides

	virtual void RecalculateBaseEyeHeight() override;

	// General

	virtual bool IsAnimMontagePlaying();

	// Aiming

	virtual void TryToggleAiming();
	virtual void StartAiming();
	virtual void StopAiming();
	virtual void OnStartAiming_Implementation();
	virtual void OnStartAimingInternal();
	virtual void OnStopAiming_Implementation();
	virtual void OnStopAimingInternal();
	UFUNCTION(Server, Reliable)
	virtual void Server_StartAiming();
	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_StartAiming();
	UFUNCTION(Server, Reliable)
	virtual void Server_StopAiming();
	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_StopAiming();

	// Reloading Weapons

	void OnReloadStarted();
	UFUNCTION(Server, Reliable)
	void Server_OnReloadStarted();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnReloadStarted();

	// Equipment Items

	void OnTogglePrimaryItem(bool bCanEquip) const;
	UFUNCTION(Server, Reliable)
	void Server_OnTogglePrimaryItem(bool bCanEquip);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnTogglePrimaryItem(bool bCanEquip);
	void OnThrowItem();
	UFUNCTION(Server, Reliable)
	void Server_ThrowItem();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ThrowItem();

	// Jumping / Landing

	UFUNCTION()
	virtual void UpdateJumpApexHeight();
	UFUNCTION()
	virtual void OnCharacterLanded(const FHitResult& Hit);
	virtual void OnHardLandStart();
	virtual void OnHardLandEnd();

	// Sprinting / Sliding / OutOfStamina

	virtual void TryChangeSprintState();
	virtual void OnSprintStart_Implementation();
	virtual void OnSprintStop_Implementation();
	virtual void OnSprintStartInternal();
	virtual void OnSprintStopInternal();
	virtual void UpdateSliding() const;
	virtual void OnOutOfStaminaStart() {};
	virtual void OnOutOfStaminaEnd() {};
	UFUNCTION()
	virtual void OnOutOfStaminaEvent(const bool bIsOutOfStamina);

	// Crouching / Proning

	virtual bool CanCrouch() const override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual bool CanProne() const;
	virtual void Prone() const;
	virtual void UnProne() const;

	// Mantling

	virtual bool CanMantle() const;
	virtual const FMantlingSettings& GetMantlingSettings(float LedgeHeight) const;
	virtual bool DetectLedge(OUT FLedgeDescription& LedgeDescription) const;
	virtual void OnMantle(const FMantlingSettings& MantlingSettings, const FMantlingMovementParameters& MantlingParameters) {};

	// Interactive Actors

	virtual bool CanInteractWithActors() const;
	virtual ALadder* GetAvailableLadder();
	virtual AZipline* GetAvailableZipline();
	virtual void OnAttachedToLadderFromTop(ALadder* Ladder) {};

	// Wall Running

	UFUNCTION()
	virtual void OnCharacterCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// Death

	UFUNCTION()
	virtual void OnDeath(bool bShouldPlayAnimMontage);
	virtual void OnDeathStarted() {};

	// Inverse Kinematics

	virtual float GetIKOffsetForSocket(const FName& SocketName) const;
	virtual float GetPelvisOffset() const;
};
