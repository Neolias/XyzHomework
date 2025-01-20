// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/XyzBaseCharacter.h"

#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Curves/CurveVector.h"
#include "Kismet/KismetSystemLibrary.h"

#include "XyzHomeworkTypes.h"
#include "Actors/Equipment/Throwables/ThrowableItem.h"
#include "Actors/Equipment/Weapons/MeleeWeaponItem.h"
#include "Actors/Equipment/Weapons/RangedWeaponItem.h"
#include "Controllers/XyzPlayerController.h"
#include "Components/MovementComponents/XyzBaseCharMovementComponent.h"
#include "Components/CharacterComponents/CharacterAttributesComponent.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "Actors/Interactive/InteractiveActor.h"
#include "Actors/Interactive/Environment/Ladder.h"
#include "Actors/Interactive/Environment/Zipline.h"

AXyzBaseCharacter::AXyzBaseCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UXyzBaseCharMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	checkf(GetCharacterMovement()->IsA<UXyzBaseCharMovementComponent>(), TEXT("AXyzBaseCharacter::AXyzBaseCharacter() should be used only with UXyzBaseCharMovementComponent"))
		BaseCharacterMovementComponent = StaticCast<UXyzBaseCharMovementComponent*>(GetCharacterMovement());

	CharacterAttributesComponent = CreateDefaultSubobject<UCharacterAttributesComponent>(TEXT("CharacterAttributes"));
	CharacterEquipmentComponent = CreateDefaultSubobject<UCharacterEquipmentComponent>(TEXT("CharacterEquipment"));

	BaseCharacterMovementComponent->CrouchedHalfHeight = 60.f;
	BaseCharacterMovementComponent->bCanWalkOffLedgesWhenCrouching = 1;
	BaseCharacterMovementComponent->Buoyancy = .9f;
	BaseCharacterMovementComponent->RotationRate = FRotator(540.f, 540.f, 540.f);
	BaseCharacterMovementComponent->bOrientRotationToMovement = 1;
	BaseCharacterMovementComponent->NavAgentProps.bCanCrouch = 1;

	ProneEyeHeight = BaseCharacterMovementComponent->GetProneCapsuleHalfHeight() * .8f;

	GetCapsuleComponent()->SetCapsuleHalfHeight(90.f);
	CrouchedEyeHeight = 32.f;

	SkeletalMeshComponent = GetMesh();
	if (IsValid(SkeletalMeshComponent))
	{
		SkeletalMeshComponent->CastShadow = true;
		SkeletalMeshComponent->bCastDynamicShadow = true;
	}

	CachedCollisionCapsuleScaledRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
	CachedCollisionCapsuleScaledHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	IKScale = GetActorScale3D().Z;
}

void AXyzBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AXyzBaseCharacter::OnCharacterCapsuleHit);
	OnReachedJumpApex.AddDynamic(this, &AXyzBaseCharacter::UpdateJumpApexHeight);
	LandedDelegate.AddDynamic(this, &AXyzBaseCharacter::OnCharacterLanded);
	CharacterAttributesComponent->OutOfStaminaEventSignature.AddDynamic(this, &AXyzBaseCharacter::OnOutOfStaminaEvent);
	CharacterAttributesComponent->OnDeathDelegate.AddDynamic(this, &AXyzBaseCharacter::OnDeath);
}

void AXyzBaseCharacter::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TryToggleWeaponFire();
	if (IsLocallyControlled())
	{
		TryToggleAiming();
		TryChangeSprintState();
	}

	UpdateSliding();

	IKLeftFootOffset = FMath::FInterpTo(IKLeftFootOffset, GetIKOffsetForSocket(LeftFootSocketName), DeltaSeconds, IKInterpSpeed);
	IKRightFootOffset = FMath::FInterpTo(IKRightFootOffset, GetIKOffsetForSocket(RightFootSocketName), DeltaSeconds, IKInterpSpeed);
	IKPelvisOffset = FMath::FInterpTo(IKPelvisOffset, GetPelvisOffset(), DeltaSeconds, IKInterpSpeed);
}

// Overrides

FRotator AXyzBaseCharacter::GetAimOffset()
{
	const FVector AimDirectionWorld = GetBaseAimRotation().Vector();
	const FVector AimDirectionLocal = GetTransform().InverseTransformVector(AimDirectionWorld);
	return AimDirectionLocal.ToOrientationRotator();
}

void AXyzBaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	XyzPlayerController = Cast<AXyzPlayerController>(NewController);

	AAIController* AIController = Cast<AAIController>(NewController);
	if (IsValid(AIController))
	{
		const FGenericTeamId TeamId = (uint8)Team;
		AIController->SetGenericTeamId(TeamId);
	}
}

void AXyzBaseCharacter::Jump()
{
	if (BaseCharacterMovementComponent->IsCrouching())
	{
		if (BaseCharacterMovementComponent->IsProne())
		{
			UnProne();
		}
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void AXyzBaseCharacter::OnMovementModeChanged(const EMovementMode PrevMovementMode, const uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	if (!BaseCharacterMovementComponent->IsFalling() && !BaseCharacterMovementComponent->IsMovingOnGround())
	{
		CharacterEquipmentComponent->UnequipCurrentItem();
	}
	else if (PrevMovementMode != MOVE_Falling && PrevMovementMode != MOVE_Walking)
	{
		CharacterEquipmentComponent->EquipPreviousItemIfUnequipped();
	}
}

void AXyzBaseCharacter::RecalculateBaseEyeHeight()
{
	if (BaseCharacterMovementComponent->IsProne() || BaseCharacterMovementComponent->IsSliding())
	{
		BaseEyeHeight = ProneEyeHeight;
	}
	else if (BaseCharacterMovementComponent->IsCrouching())
	{
		BaseEyeHeight = CrouchedEyeHeight;
	}
	else
	{
		Super::RecalculateBaseEyeHeight();
	}
}

// General

bool AXyzBaseCharacter::IsAnimMontagePlaying()
{
	if (IsValid(SkeletalMeshComponent))
	{
		const UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
		return IsValid(AnimInstance) && AnimInstance->IsAnyMontagePlaying();
	}
	return false;
}

void AXyzBaseCharacter::OnCharacterCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	BaseCharacterMovementComponent->StartWallRun(Hit);
}

// Aiming

bool AXyzBaseCharacter::CanAim()
{
	return bWantsToAim && BaseCharacterMovementComponent->IsMovingOnGround() && !BaseCharacterMovementComponent->IsProne()
		&& !BaseCharacterMovementComponent->IsCrouching() && !BaseCharacterMovementComponent->IsSprinting() && !BaseCharacterMovementComponent->IsSliding()
		&& !CharacterAttributesComponent->IsOutOfStamina() && GetRootComponent() && !GetRootComponent()->IsSimulatingPhysics();
}

void AXyzBaseCharacter::SetWantsToAim()
{
	bWantsToAim = true;
}

void AXyzBaseCharacter::StartAiming()
{
	const AEquipmentItem* EquipmentItem = CharacterEquipmentComponent->GetCurrentEquipmentItem();
	if (IsValid(EquipmentItem) && EquipmentItem->CanAimWithThisItem())
	{
		bIsAiming = true;
		BaseCharacterMovementComponent->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = true;
		CurrentAimingMovementSpeed = EquipmentItem->GetAimingWalkSpeed();
		OnStartAiming();
	}
}

void AXyzBaseCharacter::TryToggleAiming()
{
	const bool bCanAim = CanAim();

	if (!bIsAiming && bCanAim)
	{
		StartAiming();
		if (GetLocalRole() == ROLE_Authority)
		{
			Multicast_StartAiming();
		}
		else if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_StartAiming();
		}
	}
	else if (bIsAiming && !bCanAim)
	{
		StopAiming();
		if (GetLocalRole() == ROLE_Authority)
		{
			Multicast_StopAiming();
		}
		else if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_StopAiming();
		}
	}
}

void AXyzBaseCharacter::Server_StartAiming_Implementation()
{
	StartAiming();
	Multicast_StartAiming();
}

void AXyzBaseCharacter::Multicast_StartAiming_Implementation()
{
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		StartAiming();
	}
}

void AXyzBaseCharacter::OnStartAiming_Implementation()
{
	OnStartAimingInternal();
}

void AXyzBaseCharacter::OnStartAimingInternal()
{
	if (OnAimingStateChanged.IsBound())
	{
		OnAimingStateChanged.Broadcast(true);
	}
}

void AXyzBaseCharacter::ResetWantsToAim()
{
	bWantsToAim = false;
}

void AXyzBaseCharacter::StopAiming()
{
	bIsAiming = false;
	BaseCharacterMovementComponent->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
	CurrentAimingMovementSpeed = BaseCharacterMovementComponent->MaxWalkSpeed;
	OnStopAiming();
}

void AXyzBaseCharacter::Server_StopAiming_Implementation()
{
	Multicast_StopAiming();
	StopAiming();
}

void AXyzBaseCharacter::Multicast_StopAiming_Implementation()
{
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		StopAiming();
	}
}

void AXyzBaseCharacter::OnStopAiming_Implementation()
{
	OnStopAimingInternal();
}

void AXyzBaseCharacter::OnStopAimingInternal()
{
	if (OnAimingStateChanged.IsBound())
	{
		OnAimingStateChanged.Broadcast(false);
	}
}

// Shooting

bool AXyzBaseCharacter::CanFireWeapon()
{
	return bIsAiming;
}

void AXyzBaseCharacter::TryToggleWeaponFire()
{
	if (!CurrentRangedWeapon.IsValid())
	{
		return;
	}

	if (bWantsToFire && CanFireWeapon() && !CurrentRangedWeapon->IsReloading())
	{
		CurrentRangedWeapon->StartFire();
	}
	else
	{
		CurrentRangedWeapon->StopFire();
	}
}

void AXyzBaseCharacter::StartWeaponFire()
{
	CurrentRangedWeapon = CharacterEquipmentComponent->GetCurrentRangedWeapon();
	if (!CurrentRangedWeapon.IsValid())
	{
		return;
	}

	const FWeaponModeParameters* ModeParameters = CurrentRangedWeapon->GetWeaponModeParameters();
	if (!ModeParameters)
	{
		return;
	}
	if (CanFireWeapon() && ModeParameters->FireMode == EWeaponFireMode::Single)
	{
		CurrentRangedWeapon->StartFire();
	}
	else if (ModeParameters->FireMode == EWeaponFireMode::FullAuto)
	{
		bWantsToFire = true;
	}
}

void AXyzBaseCharacter::StopWeaponFire()
{
	CurrentRangedWeapon = CharacterEquipmentComponent->GetCurrentRangedWeapon();
	if (!CurrentRangedWeapon.IsValid())
	{
		return;
	}

	const FWeaponModeParameters* ModeParameters = CurrentRangedWeapon->GetWeaponModeParameters();
	if (!ModeParameters)
	{
		return;
	}
	if (ModeParameters->FireMode == EWeaponFireMode::Single)
	{
		CurrentRangedWeapon->StopFire();

	}
	else if (ModeParameters->FireMode == EWeaponFireMode::FullAuto)
	{
		bWantsToFire = false;
	}
}

void AXyzBaseCharacter::OnReloadStarted()
{
	CurrentRangedWeapon = CharacterEquipmentComponent->GetCurrentRangedWeapon();
	if (CurrentRangedWeapon.IsValid())
	{
		CurrentReloadingWalkSpeed = CurrentRangedWeapon->GetAimingWalkSpeed();
		if (CharacterEquipmentComponent->CanReloadCurrentWeapon())
		{
			CurrentRangedWeapon->StartReload();
		}
	}
}

void AXyzBaseCharacter::ReloadWeapon()
{
	OnReloadStarted();
	if (GetLocalRole() == ROLE_Authority)
	{
		Multicast_OnReloadStarted();
	}
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_OnReloadStarted();
	}
}

void AXyzBaseCharacter::Server_OnReloadStarted_Implementation()
{
	OnReloadStarted();
	Multicast_OnReloadStarted();
}

void AXyzBaseCharacter::Multicast_OnReloadStarted_Implementation()
{
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		OnReloadStarted();
	}
}

void AXyzBaseCharacter::Multicast_StartAutoReload_Implementation()
{
	OnReloadStarted();
}

void AXyzBaseCharacter::OnWeaponReloaded()
{
	CurrentReloadingWalkSpeed = BaseCharacterMovementComponent->MaxWalkSpeed;
}

// Melee weapons

void AXyzBaseCharacter::UsePrimaryMeleeAttack()
{
	if (BaseCharacterMovementComponent->IsMovingOnGround() && !CharacterEquipmentComponent->IsMeleeAttackActive())
	{
		AMeleeWeaponItem* MeleeWeaponItem = CharacterEquipmentComponent->GetCurrentMeleeWeapon();
		if (IsValid(MeleeWeaponItem))
		{
			MeleeWeaponItem->StartAttack(EMeleeAttackType::PrimaryAttack);
		}
	}
}

void AXyzBaseCharacter::UseSecondaryMeleeAttack()
{
	if (BaseCharacterMovementComponent->IsMovingOnGround() && !CharacterEquipmentComponent->IsMeleeAttackActive())
	{
		AMeleeWeaponItem* MeleeWeaponItem = CharacterEquipmentComponent->GetCurrentMeleeWeapon();
		if (IsValid(MeleeWeaponItem))
		{
			MeleeWeaponItem->StartAttack(EMeleeAttackType::SecondaryAttack);
		}
	}
}

// Equipment Items

bool AXyzBaseCharacter::CanSwitchEquipmentItem()
{
	return (BaseCharacterMovementComponent->IsFalling() || BaseCharacterMovementComponent->IsMovingOnGround()) &&
		!CharacterEquipmentComponent->IsPrimaryItemEquipped() && !CharacterEquipmentComponent->IsMeleeAttackActive()
		&& !BaseCharacterMovementComponent->IsProne() && !BaseCharacterMovementComponent->IsCrouching();
}

void AXyzBaseCharacter::DrawNextEquipmentItem()
{
	if (CanSwitchEquipmentItem())
	{
		CharacterEquipmentComponent->DrawNextItem();
	}
}

void AXyzBaseCharacter::DrawPreviousEquipmentItem()
{
	if (CanSwitchEquipmentItem())
	{
		CharacterEquipmentComponent->DrawPreviousItem();
	}
}

bool AXyzBaseCharacter::CanEquipPrimaryItem()
{
	return (BaseCharacterMovementComponent->IsFalling() || BaseCharacterMovementComponent->IsMovingOnGround())
		&& !BaseCharacterMovementComponent->IsProne() && !BaseCharacterMovementComponent->IsCrouching() && !CharacterEquipmentComponent->IsPrimaryItemEquipped();
}

void AXyzBaseCharacter::OnTogglePrimaryItem(const bool bCanEquip) const
{
	if (bCanEquip)
	{
		CharacterEquipmentComponent->EquipPrimaryItem();
	}
	else
	{
		CharacterEquipmentComponent->UnequipPrimaryItem();
	}
}

void AXyzBaseCharacter::TogglePrimaryItem()
{
	const bool bCanEquip = CanEquipPrimaryItem();
	Server_OnTogglePrimaryItem(bCanEquip);
}

void AXyzBaseCharacter::Server_OnTogglePrimaryItem_Implementation(const bool bCanEquip)
{
	Multicast_OnTogglePrimaryItem(bCanEquip);
}

void AXyzBaseCharacter::Multicast_OnTogglePrimaryItem_Implementation(const bool bCanEquip)
{
	OnTogglePrimaryItem(bCanEquip);
}

bool AXyzBaseCharacter::CanThrowItem()
{
	return BaseCharacterMovementComponent->IsMovingOnGround() && !BaseCharacterMovementComponent->IsProne() && !BaseCharacterMovementComponent->IsCrouching()
		&& !BaseCharacterMovementComponent->IsSprinting() && !BaseCharacterMovementComponent->IsSliding() && !CharacterAttributesComponent->IsOutOfStamina()
		&& GetRootComponent() && !GetRootComponent()->IsSimulatingPhysics();
}

void AXyzBaseCharacter::OnThrowItem()
{
	const AThrowableItem* CurrentThrowableItem = CharacterEquipmentComponent->GetCurrentThrowableItem();
	if (IsValid(CurrentThrowableItem))
	{
		CurrentThrowItemMovementSpeed = CurrentThrowableItem->GetThrowingWalkSpeed();
		if (GetLocalRole() == ROLE_Authority)
		{
			CharacterEquipmentComponent->ThrowItem();
		}
	}
}

void AXyzBaseCharacter::ThrowItem()
{
	if (CanThrowItem())
	{
		Server_ThrowItem();
	}
}

void AXyzBaseCharacter::Server_ThrowItem_Implementation()
{
	Multicast_ThrowItem();
}

void AXyzBaseCharacter::Multicast_ThrowItem_Implementation()
{
	OnThrowItem();
}

void AXyzBaseCharacter::ActivateNextWeaponMode()
{
	if (BaseCharacterMovementComponent->IsFalling() || BaseCharacterMovementComponent->IsMovingOnGround())
	{
		CharacterEquipmentComponent->ActivateNextWeaponMode();
	}
}


// Jumping / Landing

void AXyzBaseCharacter::UpdateJumpApexHeight()
{
	CurrentJumpApexHeight = GetActorLocation().Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

void AXyzBaseCharacter::OnCharacterLanded(const FHitResult& Hit)
{
	const float FallHeight = CurrentJumpApexHeight - Hit.ImpactPoint.Z;
	CharacterAttributesComponent->TakeFallDamage(FallHeight);

	if (CharacterAttributesComponent->IsAlive() && FallHeight >= HardLandMinHeight)
	{
		OnHardLandStart();
	}
}

void AXyzBaseCharacter::OnHardLandStart()
{
	if (!IsValid(SkeletalMeshComponent) || !IsValid(HardLandAnimMontage))
	{
		return;
	}

	bIsHardLanding = true;

	AController* CharacterController = GetController();
	if (IsValid(CharacterController))
	{
		CharacterController->SetIgnoreMoveInput(true);
	}

	UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
	if (IsValid(AnimInstance))
	{
		const float Duration = AnimInstance->Montage_Play(HardLandAnimMontage, 1.f, EMontagePlayReturnType::Duration);
		GetWorldTimerManager().SetTimer(HardLandTimer, this, &AXyzBaseCharacter::OnHardLandEnd, Duration, false);
	}
}


void AXyzBaseCharacter::OnHardLandEnd()
{
	bIsHardLanding = false;

	AController* CharacterController = GetController();
	if (IsValid(CharacterController))
	{
		CharacterController->SetIgnoreMoveInput(false);
	}
}

// Sprinting

bool AXyzBaseCharacter::CanSprint()
{
	return BaseCharacterMovementComponent->IsMovingOnGround() && !BaseCharacterMovementComponent->IsProne()
		&& !CharacterAttributesComponent->IsOutOfStamina() && !IsAnimMontagePlaying();
}

void AXyzBaseCharacter::TryChangeSprintState()
{
	if (bIsSprintRequested && CanSprint())
	{
		if (!BaseCharacterMovementComponent->IsSprinting())
		{
			if (BaseCharacterMovementComponent->IsCrouching())
			{
				UnCrouch();
			}

			if (!(BaseCharacterMovementComponent->IsCrouching() && !BaseCharacterMovementComponent->bWantsToCrouch))
			{
				BaseCharacterMovementComponent->StartSprint();
			}
		}
	}
	else if (BaseCharacterMovementComponent->IsSprinting())
	{
		BaseCharacterMovementComponent->StopSprint();
	}
}

void AXyzBaseCharacter::StartSprint()
{
	bIsSprintRequested = true;
}

void AXyzBaseCharacter::StopSprint()
{
	bIsSprintRequested = false;
}

void AXyzBaseCharacter::OnSprintStart_Implementation()
{
	OnSprintStartInternal();
}

void AXyzBaseCharacter::OnSprintStop_Implementation()
{
	OnSprintStopInternal();
}

void AXyzBaseCharacter::OnSprintStartInternal()
{
	CharacterEquipmentComponent->UnequipCurrentItem();
}

void AXyzBaseCharacter::OnSprintStopInternal()
{
	if (!BaseCharacterMovementComponent->IsSliding() && !IsAnimMontagePlaying())
	{
		CharacterEquipmentComponent->EquipPreviousItemIfUnequipped();
	}
}

// Sliding

void AXyzBaseCharacter::UpdateSliding() const
{
	if (BaseCharacterMovementComponent->IsSliding())
	{
		BaseCharacterMovementComponent->AddInputVector(GetActorForwardVector(), 1.f);
	}
}

void AXyzBaseCharacter::StartSlide()
{
	BaseCharacterMovementComponent->StartSlide();
}

void AXyzBaseCharacter::StopSlide()
{
	if (BaseCharacterMovementComponent->IsSliding())
	{
		BaseCharacterMovementComponent->StopSlide();
	}
}

void AXyzBaseCharacter::OnStartSlide(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust)
{
	if (!IsValid(SlideAnimMontage))
	{
		return;
	}

	CharacterEquipmentComponent->UnequipCurrentItem();
	RecalculateBaseEyeHeight();

	const ACharacter* DefaultChar = GetDefault<ACharacter>(GetClass());
	if (IsValid(GetMesh()) && IsValid(DefaultChar->GetMesh()))
	{
		FVector& MeshRelativeLocation = GetMesh()->GetRelativeLocation_DirectMutable();
		MeshRelativeLocation.Z = DefaultChar->GetMesh()->GetRelativeLocation().Z + HalfHeightAdjust;
		BaseTranslationOffset.Z = MeshRelativeLocation.Z;
	}
	else
	{
		BaseTranslationOffset.Z = DefaultChar->GetBaseTranslationOffset().Z + HalfHeightAdjust;
	}


	AController* CharacterController = GetController();
	if (IsValid(CharacterController))
	{
		CharacterController->SetIgnoreLookInput(true);
		CharacterController->SetIgnoreMoveInput(true);
	}

	PlayAnimMontage(SlideAnimMontage);
}

void AXyzBaseCharacter::OnStopSlide(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust)
{
	if (!BaseCharacterMovementComponent->IsSprinting())
	{
		CharacterEquipmentComponent->EquipPreviousItemIfUnequipped();
	}

	const ACharacter* DefaultChar = GetDefault<ACharacter>(GetClass());
	if (IsValid(GetMesh()) && IsValid(DefaultChar->GetMesh()))
	{
		FVector& MeshRelativeLocation = GetMesh()->GetRelativeLocation_DirectMutable();
		MeshRelativeLocation.Z = DefaultChar->GetMesh()->GetRelativeLocation().Z;
		BaseTranslationOffset.Z = MeshRelativeLocation.Z;
	}
	else
	{
		BaseTranslationOffset.Z = DefaultChar->GetBaseTranslationOffset().Z;
	}

	AController* CharacterController = GetController();
	if (IsValid(CharacterController))
	{
		CharacterController->SetIgnoreLookInput(false);
		CharacterController->SetIgnoreMoveInput(false);
	}
}

// OutOfStamina

void AXyzBaseCharacter::OnOutOfStaminaEvent(const bool bIsOutOfStamina)
{
	if (bIsOutOfStamina)
	{
		OnOutOfStaminaStart();
	}
	else
	{
		OnOutOfStaminaEnd();
	}
}

// Crouching

bool AXyzBaseCharacter::CanCrouch() const
{
	return BaseCharacterMovementComponent->IsMovingOnGround() && !BaseCharacterMovementComponent->IsSprinting() && !BaseCharacterMovementComponent->IsSliding()
		&& !BaseCharacterMovementComponent->IsProne() && !CharacterAttributesComponent->IsOutOfStamina() && Super::CanCrouch();
}

void AXyzBaseCharacter::OnStartCrouch(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	CharacterEquipmentComponent->UnequipCurrentItem();
}

void AXyzBaseCharacter::OnEndCrouch(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	if (BaseCharacterMovementComponent->IsMovingOnGround())
	{
		CharacterEquipmentComponent->EquipPreviousItemIfUnequipped();
	}
	Mantle();
}

void AXyzBaseCharacter::ChangeCrouchState()
{
	Crouch();
}

// Proning

void AXyzBaseCharacter::ChangeProneState() const
{
	if (BaseCharacterMovementComponent->IsProne())
	{
		UnProne();
	}
	else if (BaseCharacterMovementComponent->IsCrouching())
	{
		Prone();
	}
}

bool AXyzBaseCharacter::CanProne() const
{
	return BaseCharacterMovementComponent->IsCrouching() && GetRootComponent() && !GetRootComponent()->IsSimulatingPhysics();
}

void AXyzBaseCharacter::Prone() const
{
	if (CanProne())
	{
		BaseCharacterMovementComponent->SetWantsToProne(true);
	}
}

void AXyzBaseCharacter::UnProne() const
{
	BaseCharacterMovementComponent->SetWantsToProne(false);
}

void AXyzBaseCharacter::OnStartProne(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust)
{
	RecalculateBaseEyeHeight();

	if (IsValid(SkeletalMeshComponent))
	{
		FVector& MeshRelativeLocation = SkeletalMeshComponent->GetRelativeLocation_DirectMutable();
		MeshRelativeLocation.Z = SkeletalMeshComponent->GetRelativeLocation().Z + HalfHeightAdjust;
		BaseTranslationOffset.Z = MeshRelativeLocation.Z;
	}
	else
	{
		BaseTranslationOffset.Z += HalfHeightAdjust;
	}

	CharacterEquipmentComponent->UnequipCurrentItem();
}

void AXyzBaseCharacter::OnEndProne(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust)
{
	if (IsValid(SkeletalMeshComponent))
	{
		FVector& MeshRelativeLocation = SkeletalMeshComponent->GetRelativeLocation_DirectMutable();
		MeshRelativeLocation.Z = SkeletalMeshComponent->GetRelativeLocation().Z - HalfHeightAdjust;
		BaseTranslationOffset.Z = MeshRelativeLocation.Z;
	}
	else
	{
		BaseTranslationOffset.Z -= HalfHeightAdjust;
	}
}

// Mantling

bool AXyzBaseCharacter::CanMantle() const
{
	return ((BaseCharacterMovementComponent->IsMovingOnGround() && !BaseCharacterMovementComponent->IsProne()
		&& !BaseCharacterMovementComponent->IsCrouching() && !BaseCharacterMovementComponent->IsSliding()) || BaseCharacterMovementComponent->IsSwimming())
		&& GetRootComponent() && !GetRootComponent()->IsSimulatingPhysics();
}

const FMantlingSettings& AXyzBaseCharacter::GetMantlingSettings(const float LedgeHeight) const
{
	return LedgeHeight > LowMantleSettings.MaxHeight ? HighMantleSettings : LowMantleSettings;
}

bool AXyzBaseCharacter::DetectLedge(FLedgeDescription& LedgeDescription) const
{
	UWorld* World = GetWorld();
	float CurrentScaledCapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FVector CharacterBottom = GetActorLocation() - CurrentScaledCapsuleHalfHeight * FVector::UpVector;
	ECollisionChannel CollisionChannel = ECC_Climbing;
	FCollisionQueryParams CollisionParams;
	CollisionParams.bTraceComplex = true;
	CollisionParams.AddIgnoredActor(this);
	float LedgeGeometryTolerance = 5.f;

	FHitResult ForwardHitResult;
	float ForwardCollisionCapsuleHalfHeight = (HighMantleSettings.MaxHeight - LowMantleSettings.MinHeight) / 2;
	FVector ForwardStartLocation = CharacterBottom + (LowMantleSettings.MinHeight + ForwardCollisionCapsuleHalfHeight - LedgeGeometryTolerance) * FVector::UpVector;
	if (BaseCharacterMovementComponent->IsSwimming())
	{
		ForwardCollisionCapsuleHalfHeight = (HighMantleSettings.MaxHeight - CachedCollisionCapsuleScaledHalfHeight) / 2;
		ForwardStartLocation = CharacterBottom + (CurrentScaledCapsuleHalfHeight + ForwardCollisionCapsuleHalfHeight - LedgeGeometryTolerance) * FVector::UpVector;
	}
	FVector ActorForwardVector = GetActorForwardVector();
	ActorForwardVector.Z = 0.f;
	FVector ForwardEndLocation = ForwardStartLocation + ForwardTraceDistance * ActorForwardVector.GetSafeNormal();
	FCollisionShape ForwardCollisionShape = FCollisionShape::MakeCapsule(CachedCollisionCapsuleScaledRadius, ForwardCollisionCapsuleHalfHeight);

	if (World->SweepSingleByChannel(ForwardHitResult, ForwardStartLocation, ForwardEndLocation, FQuat::Identity, CollisionChannel, ForwardCollisionShape, CollisionParams, FCollisionResponseParams::DefaultResponseParam))
	{
		FHitResult DownwardHitResult;
		FCollisionShape DownwardCollisionShape = FCollisionShape::MakeSphere(CachedCollisionCapsuleScaledRadius);
		FVector DownwardStartLocation = ForwardHitResult.ImpactPoint - ForwardHitResult.ImpactNormal * MantlingDepth;
		DownwardStartLocation.Z = CharacterBottom.Z + HighMantleSettings.MaxHeight + LedgeGeometryTolerance;
		FVector DownwardEndLocation = DownwardStartLocation;
		DownwardEndLocation.Z = CharacterBottom.Z + LowMantleSettings.MinHeight - LedgeGeometryTolerance;
		if (BaseCharacterMovementComponent->IsSwimming())
		{
			DownwardEndLocation.Z = CharacterBottom.Z + CurrentScaledCapsuleHalfHeight - LedgeGeometryTolerance;
		}

		if (World->SweepSingleByChannel(DownwardHitResult, DownwardStartLocation, DownwardEndLocation, FQuat::Identity, CollisionChannel, DownwardCollisionShape, CollisionParams, FCollisionResponseParams::DefaultResponseParam))
		{
			FVector OverlapTestLocation = DownwardHitResult.ImpactPoint + (CachedCollisionCapsuleScaledHalfHeight + LedgeGeometryTolerance) * FVector::UpVector;

			if (!World->OverlapBlockingTestByProfile(OverlapTestLocation, FQuat::Identity, CollisionProfilePawn, GetCapsuleComponent()->GetCollisionShape(), CollisionParams))
			{
				LedgeDescription.TargetLocation = OverlapTestLocation - LedgeGeometryTolerance * FVector::UpVector;
				LedgeDescription.TargetRotation = FVector(-ForwardHitResult.ImpactNormal.X, -ForwardHitResult.ImpactNormal.Y, 0.f).ToOrientationRotator();
				LedgeDescription.LedgeNormal = ForwardHitResult.ImpactNormal.GetSafeNormal2D();
				LedgeDescription.LedgeActor = DownwardHitResult.Component.Get();

				return true;
			}
		}
	}

	return false;
}

void AXyzBaseCharacter::Mantle(const bool bForceMantle /*= false*/)
{
	if (CanMantle() || bForceMantle)
	{
		FLedgeDescription LedgeDescription;
		if (DetectLedge(LedgeDescription))
		{
			FMantlingMovementParameters MantlingParameters;
			MantlingParameters.InitialLocation = GetActorLocation() + (CachedCollisionCapsuleScaledHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight()) * FVector::UpVector;
			MantlingParameters.InitialRotation = GetActorRotation();
			MantlingParameters.TargetLocation = LedgeDescription.TargetLocation;
			MantlingParameters.TargetRotation = LedgeDescription.TargetRotation;
			MantlingParameters.TargetActor = LedgeDescription.LedgeActor;

			const float LedgeHeight = LedgeDescription.TargetLocation.Z - MantlingParameters.InitialLocation.Z;
			const FMantlingSettings MantlingSettings = GetMantlingSettings(LedgeHeight);
			if (!IsValid(MantlingSettings.MantlingMontage) || !IsValid(MantlingSettings.MantlingCurve))
			{
				return;
			}
			MantlingParameters.MantlingCurve = MantlingSettings.MantlingCurve;

			const float MantlingHeight = (MantlingParameters.TargetLocation - MantlingParameters.InitialLocation).Z;
			float MinRange;
			float MaxRange;
			MantlingSettings.MantlingCurve->GetTimeRange(MinRange, MaxRange);
			MantlingParameters.Duration = MaxRange - MinRange;

			const FVector2D SourceRange(MantlingSettings.MinHeight, MantlingSettings.MaxHeight);
			const FVector2D TargetRange(MantlingSettings.MinHeightStartTime, MantlingSettings.MaxHeightStartTime);
			MantlingParameters.StartTime = FMath::GetMappedRangeValueClamped(SourceRange, TargetRange, MantlingHeight);
			MantlingParameters.InitialAnimationLocation = MantlingParameters.TargetLocation - MantlingSettings.AnimationCorrectionZ * FVector::UpVector + MantlingSettings.AnimationCorrectionXY * LedgeDescription.LedgeNormal;

			if (GetLocalRole() == ROLE_Authority || IsLocallyControlled())
			{
				BaseCharacterMovementComponent->StartMantle(MantlingParameters);
			}

			if (GetLocalRole() == ROLE_Authority)
			{
				Multicast_Mantle(true);
			}

			if (IsValid(SkeletalMeshComponent))
			{
				UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
				AnimInstance->Montage_Play(MantlingSettings.MantlingMontage, 1.f, EMontagePlayReturnType::Duration, MantlingParameters.StartTime);
				OnMantle(MantlingSettings, MantlingParameters);
			}
		}
	}
}

void AXyzBaseCharacter::Multicast_Mantle_Implementation(const bool bForceMantle)
{
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		Mantle(bForceMantle);
	}
}

// Interactive Actors

bool AXyzBaseCharacter::CanInteractWithActors() const
{
	return !BaseCharacterMovementComponent->IsSliding() && !BaseCharacterMovementComponent->IsProne();
}

void AXyzBaseCharacter::RegisterInteractiveActor(AInteractiveActor* InteractiveActor)
{
	InteractiveActors.AddUnique(InteractiveActor);
}

void AXyzBaseCharacter::UnRegisterInteractiveActor(AInteractiveActor* InteractiveActor)
{
	InteractiveActors.RemoveSingleSwap(InteractiveActor);
}

void AXyzBaseCharacter::InteractWithLadder()
{
	if (!CanInteractWithActors())
	{
		return;
	}

	if (BaseCharacterMovementComponent->IsOnLadder())
	{
		BaseCharacterMovementComponent->DetachCharacterFromLadder(EDetachFromLadderMethod::JumpOff);
	}
	else
	{
		ALadder* Ladder = GetAvailableLadder();
		if (IsValid(Ladder))
		{
			if (Ladder->IsOnTop())
			{
				PlayAnimMontage(Ladder->GetAttachFromTopAnimMontage());
				OnAttachedToLadderFromTop(Ladder);
			}

			BaseCharacterMovementComponent->AttachCharacterToLadder(Ladder);
		}
	}
}

void AXyzBaseCharacter::InteractWithZipline()
{
	if (!CanInteractWithActors())
	{
		return;
	}

	if (BaseCharacterMovementComponent->IsOnZipline())
	{
		BaseCharacterMovementComponent->DetachCharacterFromZipline(EDetachFromZiplineMethod::Fall);
	}
	else
	{
		AZipline* Zipline = GetAvailableZipline();
		if (IsValid(Zipline))
		{
			BaseCharacterMovementComponent->AttachCharacterToZipline(Zipline);
		}
	}
}

ALadder* AXyzBaseCharacter::GetAvailableLadder()
{
	for (AInteractiveActor* InteractiveActor : InteractiveActors)
	{
		ALadder* Ladder = Cast<ALadder>(InteractiveActor);
		if (IsValid(Ladder))
		{
			return Ladder;
		}
	}
	return nullptr;
}

AZipline* AXyzBaseCharacter::GetAvailableZipline()
{
	for (AInteractiveActor* InteractiveActor : InteractiveActors)
	{
		AZipline* Zipline = Cast<AZipline>(InteractiveActor);
		if (IsValid(Zipline))
		{
			return Zipline;
		}
	}
	return nullptr;
}

// Wall Running

void AXyzBaseCharacter::JumpOffRunnableWall()
{
	if (BaseCharacterMovementComponent->IsWallRunning())
	{
		BaseCharacterMovementComponent->DetachCharacterFromRunnableWall(EDetachFromRunnableWallMethod::JumpOff);
	}
}

// Death

void AXyzBaseCharacter::OnDeath(const bool bShouldPlayAnimMontage)
{
	BaseCharacterMovementComponent->DisableMovement();

	if (bShouldPlayAnimMontage && IsValid(DeathAnimMontage))
	{
		const float Duration = PlayAnimMontage(DeathAnimMontage);
		if (Duration == 0.f)
		{
			EnableRagdoll();
		}
	}
	else
	{
		EnableRagdoll();
	}

	OnDeathStarted();
}

void AXyzBaseCharacter::EnableRagdoll()
{
	GetCapsuleComponent()->SetCollisionProfileName("NoCollision");
	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	if (!IsValid(SkeletalMesh))
	{
		return;
	}
	SkeletalMesh->SetCollisionProfileName(CollisionProfileRagdoll);
	SkeletalMesh->SetSimulatePhysics(true);
}

// Inverse Kinematics

float AXyzBaseCharacter::GetIKOffsetForSocket(const FName& SocketName) const
{
	float Result = 0.f;

	if (IsValid(SkeletalMeshComponent))
	{
		const FVector SocketLocation = SkeletalMeshComponent->GetSocketLocation(SocketName);
		const FVector TraceStart(SocketLocation.X, SocketLocation.Y, GetActorLocation().Z);
		const float CurrentCapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() * IKScale;
		const FVector TraceEnd = TraceStart - (CurrentCapsuleHalfHeight + IKTraceDistance) * FVector::UpVector;

		FHitResult HitResult;
		const ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECC_Visibility);
		const FVector FootBoxSize(10.f, 1.f, 5.f);
		if (UKismetSystemLibrary::BoxTraceSingle(GetWorld(), TraceStart, TraceEnd, FootBoxSize, SkeletalMeshComponent->GetSocketRotation(SocketName), TraceType, true, TArray<AActor*>(), EDrawDebugTrace::None, HitResult, true))
		{
			Result = (TraceStart.Z - CurrentCapsuleHalfHeight - HitResult.Location.Z) / IKScale;
		}
	}

	return Result;
}

float AXyzBaseCharacter::GetPelvisOffset() const
{
	if (IKLeftFootOffset > 0.f && IKRightFootOffset > 0.f)
	{
		return -FMath::Max(IKLeftFootOffset, IKRightFootOffset);
	}
	if (IKLeftFootOffset > 0.f)
	{
		return -IKLeftFootOffset;
	}
	if (IKRightFootOffset > 0.f)
	{
		return -IKRightFootOffset;
	}
	return 0.f;
}


// IGenericTeamAgentInterface
FGenericTeamId AXyzBaseCharacter::GetGenericTeamId() const
{
	return FGenericTeamId((uint8)Team);
}
// ~IGenericTeamAgentInterface
