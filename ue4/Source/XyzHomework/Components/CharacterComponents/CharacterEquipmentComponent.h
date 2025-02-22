// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Actors/Projectiles/ProjectilePool.h"
#include "Components/ActorComponent.h"
#include "Inventory/Items/InventoryItem.h"
#include "CharacterEquipmentComponent.generated.h"

class URadialMenuWidget;
class UDataTable;
class UEquipmentViewWidget;
class AXyzProjectile;
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
	const TArray<AEquipmentItem*>& GetEquippedItems() const { return EquippedItemsArray; }
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
	void CreateLoadout();
	void EquipFromDefaultItemSlot(const bool bShouldSkipAnimation = true);
	void DrawNextItem();
	void DrawPreviousItem();
	UFUNCTION(BlueprintCallable, Category = "Character Equipment Component")
	bool EquipItemBySlotType(EEquipmentItemSlot EquipmentItemSlot, bool bShouldSkipAnimation = true);
	void UnequipCurrentItem();
	void EquipPreviousItemIfUnequipped(bool bShouldSkipAnimation = true);
	void AttachCurrentEquippedItemToCharacterMesh();
	void ActivateNextWeaponMode();
	bool CanReloadCurrentWeapon();
	void TryReloadNextBullet();
	bool IsCurrentWeaponMagazineFull() const;
	void EquipPrimaryItem(const bool bForceEquip = false);
	UFUNCTION()
	void UnequipPrimaryItem(const bool bForceUnequip = false);
	bool CanThrowItem(AThrowableItem* ThrowableItem);
	void ThrowItem();
	void SetAmmo(EWeaponAmmoType AmmoType, int32 NewAmmo);
	bool AddAmmo(EWeaponAmmoType AmmoType, int32 Amount);
	int32 RemoveAmmo(EWeaponAmmoType AmmoType, int32 Amount);

	// Equipment widgets

	void SetInventoryItemDataTable(const TSoftObjectPtr<UDataTable>& NewDataTable) { InventoryItemDataTable = NewDataTable; }
	bool AddEquipmentItem(TSubclassOf<AEquipmentItem> EquipmentItemClass, int32 Amount = 1, int32 EquipmentSlotIndex = -1);
	bool RemoveEquipmentItem(int32 EquipmentSlotIndex);
	void OpenViewEquipment(APlayerController* PlayerController);
	void CloseViewEquipment();
	bool IsViewEquipmentVisible() const;
	void OpenRadialMenu(APlayerController* PlayerController);
	void CloseRadialMenu();
	bool IsRadialMenuVisible();
	void OnEquipmentSlotUpdated(int32 SlotIndex);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment | Loadout")
	EEquipmentItemSlot DefaultEquipmentItemSlot = EEquipmentItemSlot::PrimaryWeapon;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment | Loadout")
	TMap<EWeaponAmmoType, int32> MaxEquippedWeaponAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment | Loadout")
	TMap<EEquipmentItemSlot, TSubclassOf<AEquipmentItem>> EquipmentSlots;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment | Loadout")
	TArray<EEquipmentItemSlot> WeaponSwitchIgnoredSlots;
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment | Loadout")
	TArray<FProjectilePool> ProjectilePools;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment | Items")
	TSubclassOf<UEquipmentViewWidget> EquipmentViewWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment | Items")
	TSubclassOf<URadialMenuWidget> RadialMenuWidgetClass;

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

	UPROPERTY()
	UEquipmentViewWidget* EquipmentViewWidget;
	UPROPERTY()
	URadialMenuWidget* RadialMenuWidget;
	TSoftObjectPtr<UDataTable> InventoryItemDataTable;

	virtual void BeginPlay() override;
	void InstantiateProjectilePools(AActor* Owner);
	UFUNCTION()
	void OnCurrentWeaponAmmoChanged(int32 NewAmmo);
	UFUNCTION()
	void OnCurrentWeaponReloaded();
	UFUNCTION()
	void OnWeaponMagazineEmpty();
	UFUNCTION()
	void OnThrowItemEnd();
	UFUNCTION()
	void OnThrowItemAnimationFinished();
	void OnCurrentThrowableAmmoChanged(int32 NewAmmo0) const;
	void LoadoutOneItem(EEquipmentItemSlot EquipmentSlot, TSubclassOf<AEquipmentItem> EquipmentItemClass, USkeletalMeshComponent* SkeletalMesh, int32 CountInSlot = -1);
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

	// Equipment Widgets

	void CreateEquipmentViewWidget(APlayerController* PlayerController);
	void CreateRadialMenuWidget(APlayerController* PlayerController);
	void InitializeInventoryItem(AEquipmentItem* EquipmentItem, int32 Count = 1) const;
	EEquipmentItemSlot FindCompatibleSlot(AEquipmentItem* EquipmentItem);
};
