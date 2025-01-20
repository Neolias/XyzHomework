// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Equipment/Weapons/RangedWeaponItem.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

#include "Characters/PlayerCharacter.h"
#include "GameFramework/Character.h"
#include "Components/WeaponComponents/WeaponMuzzleComponent.h"
#include "Net/UnrealNetwork.h"

ARangedWeaponItem::ARangedWeaponItem()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("WeaponMeshComponent");
	MeshComponent->SetupAttachment(RootComponent);

	MuzzleComponent = CreateDefaultSubobject<UWeaponMuzzleComponent>("WeaponMuzzleComponent");
	MuzzleComponent->SetupAttachment(MeshComponent, MuzzleSocketName);

	EquipmentItemType = EEquipmentItemType::Pistol;
	ReticleType = EReticleType::Default;

	CurrentWeaponAmmo.AddZeroed((uint32)EWeaponAmmoType::Max);
}

void ARangedWeaponItem::BeginPlay()
{
	Super::BeginPlay();

	Loadout();
}

void ARangedWeaponItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ARangedWeaponItem, CurrentWeaponAmmo);
	DOREPLIFETIME(ARangedWeaponItem, CurrentWeaponModeIndex);
	DOREPLIFETIME(ARangedWeaponItem, bIsReloading);
}

void ARangedWeaponItem::Loadout()
{
	if (!IsCurrentWeaponModeValid())
	{
		WeaponModes.Add(FWeaponModeParameters());
		SetCurrentWeaponMode(0);
	}
}

bool ARangedWeaponItem::IsCurrentWeaponModeValid()
{
	if (!CurrentWeaponMode)
	{
		if (WeaponModes.Num() < 1)
		{
			return false;
		}
		SetCurrentWeaponMode(DefaultWeaponModeIndex);
		return true;
	}
	return true;
}

const FWeaponModeParameters* ARangedWeaponItem::GetWeaponModeParameters(const int32 ModeIndex/* = -1*/) const
{
	if (WeaponModes.Num() < 1 || WeaponModes.Num() - 1 < ModeIndex)
	{
		return nullptr;
	}

	if (ModeIndex == -1)
	{
		return &WeaponModes[0];
	}

	return &WeaponModes[ModeIndex];
}

void ARangedWeaponItem::SetCurrentWeaponMode(const int32 ModeIndex)
{
	if (WeaponModes.Num() < 1)
	{
		WeaponModes.Add(FWeaponModeParameters());
		CurrentWeaponModeIndex = 0;
	}
	else if (WeaponModes.Num() - 1 >= ModeIndex)
	{
		CurrentWeaponModeIndex = ModeIndex;
	}
	else
	{
		CurrentWeaponModeIndex = 0;
	}

	CurrentWeaponMode = &WeaponModes[CurrentWeaponModeIndex];

	if (CachedBaseCharacterOwner.IsValid() && CachedBaseCharacterOwner->GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_SetCurrentWeaponMode(CurrentWeaponModeIndex);
	}
}

void ARangedWeaponItem::Server_SetCurrentWeaponMode_Implementation(const int32 ModeIndex)
{
	SetCurrentWeaponMode(ModeIndex);
}

void ARangedWeaponItem::OnRep_CurrentWeaponModeIndex()
{
	if (WeaponModes.Num() > CurrentWeaponModeIndex)
	{
		CurrentWeaponMode = &WeaponModes[CurrentWeaponModeIndex];
	}
	else if (CachedBaseCharacterOwner.IsValid() && CachedBaseCharacterOwner->GetLocalRole() == ROLE_AutonomousProxy)
	{
		SetCurrentWeaponMode(0);
	}
}

void ARangedWeaponItem::SetCurrentAmmo(const int32 AmmoAmount)
{
	if (!CurrentWeaponMode || !(CachedBaseCharacterOwner.IsValid() && CachedBaseCharacterOwner->GetLocalRole() == ROLE_Authority))
	{
		return;
	}

	CurrentWeaponAmmo[(int32)CurrentWeaponMode->AmmoType] = AmmoAmount;
	if (OnAmmoChanged.IsBound())
	{
		OnAmmoChanged.Broadcast(AmmoAmount);
	}
}

void ARangedWeaponItem::OnRep_CurrentWeaponAmmo(TArray<int32> CurrentWeaponAmmo_Old)
{
	if (OnAmmoChanged.IsBound())
	{
		for (int i = 0; i < CurrentWeaponAmmo.Num(); ++i)
		{
			if (CurrentWeaponAmmo[i] == CurrentWeaponAmmo_Old[i])
			{
				continue;
			}
			OnAmmoChanged.Broadcast(CurrentWeaponAmmo[i]);
		}
	}
}

bool ARangedWeaponItem::CanFire() const
{
	return bIsEquipped && !bIsFiring && CurrentWeaponMode && CurrentWeaponAmmo[(int32)CurrentWeaponMode->AmmoType] > 0;
}

void ARangedWeaponItem::StartFire()
{
	if (!CachedBaseCharacterOwner.IsValid() || !CanFire())
	{
		return;
	}

	if (CachedBaseCharacterOwner->IsLocallyControlled())
	{
		const bool bIsLooping = CurrentWeaponMode->FireMode == EWeaponFireMode::FullAuto ? true : false;
		GetWorld()->GetTimerManager().ClearTimer(OneShotTimer);
		GetWorld()->GetTimerManager().SetTimer(OneShotTimer, [=]() { MakeOneShot(); }, 60.f / CurrentWeaponMode->FireRate, bIsLooping, CurrentWeaponMode->FirstShotDelay);
	}

	bIsFiring = true;
}

void ARangedWeaponItem::StopFire()
{
	if (CachedBaseCharacterOwner.IsValid() && CachedBaseCharacterOwner->IsLocallyControlled())
	{
		if (CurrentWeaponMode->FireMode == EWeaponFireMode::FullAuto)
		{
			bIsFiring = false;
			GetWorld()->GetTimerManager().ClearTimer(OneShotTimer);
		}
	}
	else
	{
		bIsFiring = false;
	}
}

void ARangedWeaponItem::OnMakeOneShot(const TArray<FShotInfo>& ShotInfoArray)
{
	if (!CachedBaseCharacterOwner.IsValid() || !CurrentWeaponMode)
	{
		return;
	}

	if (CurrentWeaponAmmo[(int32)CurrentWeaponMode->AmmoType] <= 0)
	{
		return;
	}

	EndReload(false);
	SetCurrentAmmo(CurrentWeaponAmmo[(int32)CurrentWeaponMode->AmmoType] - 1);

	if (IsValid(CharacterFireMontage))
	{
		CachedBaseCharacterOwner->PlayAnimMontage(CharacterFireMontage);
	}

	UAnimInstance* WeaponAnimInstance = MeshComponent->GetAnimInstance();
	if (IsValid(WeaponAnimInstance) && IsValid(WeaponFireMontage))
	{
		WeaponAnimInstance->Montage_Play(WeaponFireMontage);
	}

	UNiagaraSystem* MuzzleFlashFX = CurrentWeaponMode->MuzzleFlashFX;
	if (IsValid(MuzzleFlashFX) && CurrentWeaponMode->FireMode == EWeaponFireMode::Single)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), MuzzleFlashFX, MuzzleComponent->GetComponentLocation());
	}

	for (const FShotInfo& ShotInfo : ShotInfoArray)
	{
		if (IsValid(MuzzleFlashFX) && CurrentWeaponMode->FireMode == EWeaponFireMode::FullAuto)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), MuzzleFlashFX, MuzzleComponent->GetComponentLocation());
		}

		MuzzleComponent->Shoot(CurrentWeaponMode, ShotInfo.GetLocation(), ShotInfo.GetDirection().ToOrientationRotator());
	}
	if (CurrentWeaponMode->FireMode == EWeaponFireMode::Single)
	{
		GetWorld()->GetTimerManager().SetTimer(OnShotEndTimer, this, &ARangedWeaponItem::OnShotEnd, 60.f / CurrentWeaponMode->FireRate, false);
	}

	if (CachedBaseCharacterOwner->GetLocalRole() == ROLE_Authority && CurrentWeaponAmmo[(int32)CurrentWeaponMode->AmmoType] <= 0 && OnMagazineEmpty.IsBound())
	{
		OnMagazineEmpty.Broadcast();
	}
}

void ARangedWeaponItem::MakeOneShot()
{
	if (!CachedBaseCharacterOwner.IsValid() || !CurrentWeaponMode)
	{
		return;
	}

	const AController* Controller = CachedBaseCharacterOwner->GetController();
	if (!IsValid(Controller))
	{
		return;
	}

	FVector ViewPointLocation;
	FRotator ViewPointRotation;
	Controller->GetPlayerViewPoint(ViewPointLocation, ViewPointRotation);
	TArray<FShotInfo> ShotInfoArray;
	if (CachedBaseCharacterOwner->IsLocallyControlled())
	{
		for (int32 Ammo = 0; Ammo < CurrentWeaponMode->AmmoPerShot; Ammo++)
		{
			const FRotator ShotDirection = GetShotDirection(ViewPointRotation);
			ShotInfoArray.Emplace(ViewPointLocation, ShotDirection.Vector());
		}
	}

	OnMakeOneShot(ShotInfoArray);
	if (CachedBaseCharacterOwner->GetLocalRole() == ROLE_Authority)
	{
		Multicast_OnMakeOneShot(ShotInfoArray);
	}
	if (CachedBaseCharacterOwner->GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_OnMakeOneShot(ShotInfoArray);
	}
}

void ARangedWeaponItem::Server_OnMakeOneShot_Implementation(const TArray<FShotInfo>& ShotInfoArray)
{
	OnMakeOneShot(ShotInfoArray);
	Multicast_OnMakeOneShot(ShotInfoArray);
}

void ARangedWeaponItem::Multicast_OnMakeOneShot_Implementation(const TArray<FShotInfo>& ShotInfoArray)
{
	if (CachedBaseCharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		OnMakeOneShot(ShotInfoArray);
	}
}

void ARangedWeaponItem::OnShotEnd()
{
	bIsFiring = false;
}

FRotator ARangedWeaponItem::GetShotDirection(const FRotator ViewPointRotation) const
{
	const float MaxSpreadAngle = CurrentWeaponMode ? CurrentWeaponMode->MaxBulletSpreadAngle : 0.f;

	FRotator ShotDirection = ViewPointRotation;
	const float SpreadAngle = FMath::FRandRange(0.f, MaxSpreadAngle);
	const float SpreadRoll = FMath::FRandRange(0.f, 360.f);
	ShotDirection.Yaw += SpreadAngle * FMath::Cos(SpreadRoll);
	ShotDirection.Pitch += SpreadAngle * FMath::Sin(SpreadRoll);
	return ShotDirection;
}

FTransform ARangedWeaponItem::GetForeGripSocketTransform() const
{
	FTransform ForeGripTransform = MeshComponent->GetSocketTransform(ForeGripSocketName, ERelativeTransformSpace::RTS_World);
	ForeGripTransform.SetLocation(ForeGripTransform.GetLocation() + GetActorRotation().RotateVector(ForeGripOffsetFromSocket));
	return ForeGripTransform;
}

void ARangedWeaponItem::OnRep_IsReloading()
{
	if (bIsReloading)
	{
		StartReload();
	}
	else
	{
		EndReload(false);
		if (OnWeaponReloaded.IsBound())
		{
			OnWeaponReloaded.Broadcast();
		}
	}
}

void ARangedWeaponItem::StartAutoReload() const
{
	if (CachedBaseCharacterOwner.IsValid() && CachedBaseCharacterOwner->GetLocalRole() == ROLE_Authority)
	{
		CachedBaseCharacterOwner->Multicast_StartAutoReload();
	}
}

void ARangedWeaponItem::StartReload()
{
	if (!CachedBaseCharacterOwner.IsValid())
	{
		return;
	}

	bIsReloading = true;
	StopFire();

	if (CurrentWeaponMode && IsValid(CurrentWeaponMode->IronsightsCharacterReloadAnimMontage))
	{
		const float Duration = CachedBaseCharacterOwner->PlayAnimMontage(CurrentWeaponMode->IronsightsCharacterReloadAnimMontage);

		UAnimInstance* WeaponAnimInstance = MeshComponent->GetAnimInstance();
		if (IsValid(WeaponAnimInstance) && IsValid(CurrentWeaponMode->IronsightsWeaponReloadAnimMontage))
		{
			WeaponAnimInstance->Montage_Play(CurrentWeaponMode->IronsightsWeaponReloadAnimMontage);
		}

		if (CurrentWeaponMode->ReloadType == EWeaponReloadType::ByClip)
		{
			GetWorld()->GetTimerManager().SetTimer(ReloadTimer, [this]() {EndReload(true); }, Duration, false);
		}
		return;
	}

	EndReload(true);
}

void ARangedWeaponItem::EndReload(const bool bIsSuccess)
{
	if (!CachedBaseCharacterOwner.IsValid())
	{
		return;
	}

	if (!bIsSuccess && CurrentWeaponMode)
	{
		if (IsValid(CurrentWeaponMode->IronsightsCharacterReloadAnimMontage))
		{
			CachedBaseCharacterOwner->StopAnimMontage(CurrentWeaponMode->IronsightsCharacterReloadAnimMontage);
		}

		UAnimInstance* WeaponAnimInstance = MeshComponent->GetAnimInstance();
		if (IsValid(WeaponAnimInstance) && IsValid(CurrentWeaponMode->IronsightsWeaponReloadAnimMontage))
		{
			WeaponAnimInstance->Montage_Stop(0.f, CurrentWeaponMode->IronsightsWeaponReloadAnimMontage);
		}
	}

	if (bIsSuccess && OnWeaponReloaded.IsBound())
	{
		OnWeaponReloaded.Broadcast();
	}

	if (CurrentWeaponMode && CurrentWeaponMode->ReloadType == EWeaponReloadType::ByClip)
	{
		GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);
	}
	bIsReloading = false;
}
