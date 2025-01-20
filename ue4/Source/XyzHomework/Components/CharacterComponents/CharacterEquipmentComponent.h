// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Components/ActorComponent.h"
#include "CharacterEquipmentComponent.generated.h"

class AXyzProjectile;
struct FProjectilePool;
class AMeleeWeaponItem;
class AEquipmentItem;
class ARangedWeaponItem;
class AThrowableItem;

typedef TArray<AEquipmentItem*, TInlineAllocator<(uint32)EEquipmentItemSlot::Max>> TEquippedItemArray;
typedef TArray<uint32, TInlineAllocator<(uint32)EWeaponAmmoType::Max>> TEquipmentAmmoArray;
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCurrentWeaponAmmoChangedEvent, int32, int32)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCurrentThrowableAmmoChangedEvent, int32)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEquipmentItemChangedEvent, const AEquipmentItem*)

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class XYZHOMEWORK_API UCharacterEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	FOnCurrentWeaponAmmoChangedEvent OnCurrentWeaponAmmoChangedEvent;
	FOnCurrentThrowableAmmoChangedEvent OnCurrentThrowableAmmoChangedEvent;
	FOnEquipmentItemChangedEvent OnEquipmentItemChangedEvent;

	UCharacterEquipmentComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	AEquipmentItem* GetCurrentEquipmentItem() const { return CurrentEquippedItem.Get(); }
	ARangedWeaponItem* GetCurrentRangedWeapon() const { return CurrentRangedWeapon.Get(); }
	EEquipmentItemType GetCurrentRangedWeaponType() const;
	bool IsThrowingItem() const;
	bool IsReloadingWeapon() const;
	bool IsFiringWeapon() const;
	bool IsPrimaryItemEquipped() const { return bIsPrimaryItemEquipped; }
	AThrowableItem* GetCurrentThrowableItem() const { return CurrentThrowableItem.Get(); }
	AMeleeWeaponItem* GetCurrentMeleeWeapon() const { return CurrentMeleeWeapon.Get(); }
	int32 GetAvailableAmmoForWeaponMagazine(ARangedWeaponItem* RangedWeaponItem);
	UFUNCTION(BlueprintCallable, Category = "Character Equipment Component")
	EEquipmentItemSlot GetDefaultEquipmentItemSlot() const { return DefaultEquipmentItemSlot; }
	bool IsMeleeAttackActive() const { return bIsMeleeAttackActive; }
	UFUNCTION()
	void SetIsMeleeAttackActive(const bool bIsMeleeAttackActive_In) { bIsMeleeAttackActive = bIsMeleeAttackActive_In; }
	void EquipFromDefaultItemSlot(const bool bShouldSkipAnimation = true);
	void DrawNextItem();
	void DrawPreviousItem();
	UFUNCTION(BlueprintCallable, Category = "Character Equipment Component")
	bool EquipItemBySlotType(EEquipmentItemSlot EquipmentItemSlot, bool bShouldSkipAnimation = true);
	void UnequipCurrentItem();
	void EquipPreviousItemIfUnequipped();
	void AttachCurrentEquippedItemToCharacterMesh();
	void ActivateNextWeaponMode();
	bool CanReloadCurrentWeapon();
	void TryReloadNextBullet();
	bool IsCurrentWeaponMagazineFull() const;
	void EquipPrimaryItem(const bool bForceEquip = false);
	UFUNCTION()
	void UnequipPrimaryItem(const bool bForceUnequip = false);
	bool CanThrowItem(const AThrowableItem* ThrowableItem);
	void ThrowItem();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	EEquipmentItemSlot DefaultEquipmentItemSlot = EEquipmentItemSlot::PrimaryWeapon;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	TMap<EWeaponAmmoType, int32> MaxEquippedWeaponAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	TMap<EEquipmentItemSlot, TSubclassOf<AEquipmentItem>> EquipmentSlots;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	TArray<EEquipmentItemSlot> WeaponSwitchIgnoredSlots;
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Loadout")
	TArray<FProjectilePool> ProjectilePools;

	UPROPERTY()
	class AXyzBaseCharacter* BaseCharacter;
	TWeakObjectPtr<AEquipmentItem> CurrentEquippedItem;
	TWeakObjectPtr<ARangedWeaponItem> CurrentRangedWeapon;
	TWeakObjectPtr<AThrowableItem> CurrentThrowableItem;
	TWeakObjectPtr<AMeleeWeaponItem> CurrentMeleeWeapon;
	UPROPERTY(ReplicatedUsing = OnRep_CurrentSlotIndex)
	int32 CurrentSlotIndex = 0;
	UFUNCTION()
	void OnRep_CurrentSlotIndex(int32 CurrentSlotIndex_Old);
	UPROPERTY(ReplicatedUsing = OnRep_EquippedItemsArray)
	TArray<AEquipmentItem*> EquippedItemsArray;
	UFUNCTION()
	void OnRep_EquippedItemsArray();
	UPROPERTY(ReplicatedUsing = OnRep_EquipmentAmmoArray)
	TArray <uint32> EquipmentAmmoArray;
	UFUNCTION()
	void OnRep_EquipmentAmmoArray();
	FDelegateHandle OnAmmoChangedDelegate;
	FDelegateHandle OnWeaponReloadedDelegate;
	FDelegateHandle OnWeaponMagazineEmptyDelegate;
	FDelegateHandle OnItemThrownDelegate;
	FDelegateHandle OnItemThrownAnimationFinishedDelegate;
	FDelegateHandle OnMeleeAttackActivated;

	FTimerHandle EquipItemTimer;
	FTimerHandle ThrowItemTimer;
	UPROPERTY(ReplicatedUsing = OnRep_IsPrimaryItemEquipped)
	bool bIsPrimaryItemEquipped = false;
	UFUNCTION()
	void OnRep_IsPrimaryItemEquipped();
	bool bIsMeleeAttackActive = false;

	virtual void BeginPlay() override;
	void InstantiateProjectilePools(AActor* Owner);
	UFUNCTION()
	void OnCurrentWeaponAmmoChanged(int32 AmmoAmount = 0.f);
	UFUNCTION()
	void OnCurrentWeaponReloaded();
	UFUNCTION()
	void OnWeaponMagazineEmpty();
	UFUNCTION()
	void OnThrowItemEnd();
	UFUNCTION()
	void OnThrowItemAnimationFinished();
	void OnCurrentThrowableAmmoChanged(int32 NewAmmo) const;
	void CreateLoadout();
	bool IncrementCurrentSlotIndex();
	bool DecrementCurrentSlotIndex();

	UFUNCTION(Server, Reliable)
	void Server_EquipItemBySlotType(EEquipmentItemSlot EquipmentItemSlot);
	AEquipmentItem* GetNextItem();
	AEquipmentItem* GetPreviousItem();
	void UnequipItem(AEquipmentItem* EquipmentItem);
	void EquipItem(AEquipmentItem* EquipmentItem, bool bShouldSkipAnimation = false);
	void JumpToAnimMontageSection(FName ReloadLoopStartSectionName) const;
	void LoadWeaponMagazineByBullet(ARangedWeaponItem* RangedWeaponItem);
	void UpdateAmmoHUDWidgets();
	UFUNCTION(Server, Reliable)
	void Server_OnThrowItem(AXyzProjectile* ThrowableProjectile, const FVector ResetLocation);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnThrowItem(AXyzProjectile* ThrowableProjectile, const FVector ResetLocation);
};
