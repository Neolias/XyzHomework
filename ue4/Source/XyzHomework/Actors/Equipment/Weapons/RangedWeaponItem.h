// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericStructs.h"
#include "Actors/Equipment/EquipmentItem.h"
#include "RangedWeaponItem.generated.h"

class UWeaponMuzzleComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAmmoChanged, int32)
DECLARE_MULTICAST_DELEGATE(FOnWeaponReloaded)
DECLARE_MULTICAST_DELEGATE(FOnMagazineEmpty)

USTRUCT(BlueprintType)
struct FShotInfo
{
	GENERATED_BODY()

	FShotInfo() : Location_Mul_10(FVector::ZeroVector), Direction(FVector::ZeroVector) {};
	FShotInfo(FVector Location, FVector Direction) : Location_Mul_10(Location * 10.0f), Direction(Direction) {};

	UPROPERTY()
	FVector_NetQuantize100 Location_Mul_10;

	UPROPERTY()
	FVector_NetQuantizeNormal Direction;

	FVector GetLocation() const { return Location_Mul_10 * 0.1f; }
	FVector GetDirection() const { return Direction; }
};

UCLASS()
class XYZHOMEWORK_API ARangedWeaponItem : public AEquipmentItem
{
	GENERATED_BODY()

public:
	FOnAmmoChanged OnAmmoChanged;
	FOnWeaponReloaded OnWeaponReloaded;
	FOnMagazineEmpty OnMagazineEmpty;

	ARangedWeaponItem();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	const FWeaponModeParameters* GetWeaponModeParameters(int32 ModeIndex = -1) const;
	const TArray<FWeaponModeParameters>* GetWeaponModesArray() const { return &WeaponModes; }
	int32 GetCurrentWeaponModeIndex() const { return CurrentWeaponModeIndex; }
	int32 GetDefaultWeaponModeIndex() const { return DefaultWeaponModeIndex; }
	void SetCurrentWeaponMode(int32 ModeIndex);
	USkeletalMeshComponent* GetMesh() const { return MeshComponent; }
	UWeaponMuzzleComponent* GetMuzzleComponent() const { return MuzzleComponent; }
	bool IsReloading() const { return bIsReloading; }
	EWeaponAmmoType GetAmmoType() { return IsCurrentWeaponModeValid() ? CurrentWeaponMode->AmmoType : EWeaponAmmoType::None; }
	int32 GetMagazineSize() { return IsCurrentWeaponModeValid() ? CurrentWeaponMode->MagazineSize : 1; }
	EWeaponReloadType GetReloadType() { return IsCurrentWeaponModeValid() ? CurrentWeaponMode->ReloadType : EWeaponReloadType::ByClip; }
	FName GetReloadLoopStartSectionName() { return IsCurrentWeaponModeValid() ? CurrentWeaponMode->ReloadLoopStartSectionName : FName("ReloadLoopStart"); }
	FName GetReloadEndSectionName() { return IsCurrentWeaponModeValid() ? CurrentWeaponMode->ReloadEndSectionName : FName("ReloadEnd"); }
	UAnimMontage* GetWeaponReloadAnimMontage() { return IsCurrentWeaponModeValid() ? CurrentWeaponMode->IronsightsWeaponReloadAnimMontage : nullptr; }
	UAnimMontage* GetCharacterReloadAnimMontage() { return IsCurrentWeaponModeValid() ? CurrentWeaponMode->IronsightsCharacterReloadAnimMontage : nullptr; }
	int32 GetCurrentAmmo() { return IsCurrentWeaponModeValid() ? CurrentWeaponAmmo[(int32)CurrentWeaponMode->AmmoType] : 0; }
	void SetCurrentAmmo(int32 AmmoAmount);
	bool IsFiring() const { return bIsFiring; }
	bool CanFire() const;
	void StartFire();
	void StopFire();
	void MakeOneShot();
	FRotator GetShotDirection(OUT FRotator ViewPointRotation) const;
	FTransform GetForeGripSocketTransform() const;
	float GetReloadingWalkSpeed() const { return ReloadingWalkSpeed; }
	void StartAutoReload() const;
	void StartReload();
	void EndReload(bool bIsSuccess);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Parameters | Modes", meta = (ClampMin = 0, UIMin = 0))
	int32 DefaultWeaponModeIndex = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Parameters | Modes")
	TArray<FWeaponModeParameters> WeaponModes;
	UPROPERTY(EditAnywhere, Category = "Weapon Parameters | Reloading", meta = (ClampMin = 0.f, UIMin = 0.f))
	float ReloadingWalkSpeed = 200.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Parameters | Mesh")
	USkeletalMeshComponent* MeshComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Parameters | Mesh")
	FName ForeGripSocketName = "ForeGripSocket";
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Parameters | Mesh")
	FVector ForeGripOffsetFromSocket = FVector(9.f, 0.f, 0.f);
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Parameters | Muzzle Component")
	UWeaponMuzzleComponent* MuzzleComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Parameters | Muzzle Component")
	FName MuzzleSocketName = "MuzzleSocket";
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Parameters | Animations")
	UAnimMontage* CharacterFireMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Parameters | Animations")
	UAnimMontage* WeaponFireMontage;

	FTimerHandle OneShotTimer;
	FTimerHandle ReloadTimer;
	FTimerHandle OnShotEndTimer;
	UPROPERTY(ReplicatedUsing = OnRep_IsReloading)
	bool bIsReloading = false;
	UFUNCTION()
	void OnRep_IsReloading();
	bool bIsFiring = false;
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeaponModeIndex)
	int32 CurrentWeaponModeIndex = 0;
	UFUNCTION()
	void OnRep_CurrentWeaponModeIndex();
	const FWeaponModeParameters* CurrentWeaponMode;
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeaponAmmo)
	TArray<int32> CurrentWeaponAmmo;
	UFUNCTION()
	void OnRep_CurrentWeaponAmmo(TArray<int32> CurrentWeaponAmmo_Old);

	void Loadout();
	bool IsCurrentWeaponModeValid();
	void OnMakeOneShot(const TArray<FShotInfo>& ShotInfoArray);
	void OnShotEnd();
	UFUNCTION(Server, Reliable)
	void Server_OnMakeOneShot(const TArray<FShotInfo>& ShotInfoArray);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnMakeOneShot(const TArray<FShotInfo>& ShotInfoArray);
	UFUNCTION(Server, Reliable)
	void Server_SetCurrentWeaponMode(const int32 ModeIndex);
};
