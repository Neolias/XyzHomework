// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

#include "Actors/Equipment/Throwables/ThrowableItem.h"
#include "Actors/Equipment/Weapons/MeleeWeaponItem.h"
#include "Characters/XyzBaseCharacter.h"
#include "Actors/Equipment/Weapons/RangedWeaponItem.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/Items/InventoryItem.h"
#include "Net/UnrealNetwork.h"
#include "UI/Widgets/Equipment/EquipmentViewWidget.h"
#include "Actors/Projectiles/ProjectilePool.h"

UCharacterEquipmentComponent::UCharacterEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	EquipmentAmmoArray.AddZeroed((uint32)EWeaponAmmoType::Max);
	EquippedItemsArray.AddZeroed((uint32)EEquipmentItemSlot::Max);
}

void UCharacterEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCharacterEquipmentComponent, CurrentSlotIndex);
	DOREPLIFETIME(UCharacterEquipmentComponent, EquippedItemsArray);
	DOREPLIFETIME(UCharacterEquipmentComponent, EquipmentAmmoArray);
	DOREPLIFETIME(UCharacterEquipmentComponent, bIsPrimaryItemEquipped);
	DOREPLIFETIME(UCharacterEquipmentComponent, ProjectilePools);
}

void UCharacterEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	checkf(GetOwner()->IsA<AXyzBaseCharacter>(), TEXT("The component owner can be only AXyzBaseCharacter"))
		BaseCharacter = StaticCast<AXyzBaseCharacter*>(GetOwner());

	if (BaseCharacter->GetRemoteRole() != ROLE_Authority)
	{
		InstantiateProjectilePools(BaseCharacter);
	}
	if (BaseCharacter->IsLocallyControlled())
	{
		UpdateAmmoHUDWidgets();
	}
}

void UCharacterEquipmentComponent::InstantiateProjectilePools(AActor* Owner)
{
	for (FProjectilePool& Pool : ProjectilePools)
	{
		Pool.InstantiatePool(GetWorld(), Owner);
	}
}

bool UCharacterEquipmentComponent::IsThrowingItem() const
{
	return CurrentThrowableItem.IsValid() ? CurrentThrowableItem->IsThrowing() : false;
}

bool UCharacterEquipmentComponent::IsReloadingWeapon() const
{
	return CurrentRangedWeapon.IsValid() ? CurrentRangedWeapon->IsReloading() : false;
}

bool UCharacterEquipmentComponent::IsFiringWeapon() const
{
	return CurrentRangedWeapon.IsValid() ? CurrentRangedWeapon->IsFiring() : false;
}

EEquipmentItemType UCharacterEquipmentComponent::GetCurrentRangedWeaponType() const
{
	if (CurrentRangedWeapon.IsValid())
	{
		return CurrentRangedWeapon->GetEquipmentItemType();
	}
	return EEquipmentItemType::None;
}

void UCharacterEquipmentComponent::ActivateNextWeaponMode()
{
	if (CurrentRangedWeapon.IsValid())
	{
		UAnimMontage* EquipAnimMontage = CurrentRangedWeapon->GetEquipItemAnimMontage();
		if (IsValid(EquipAnimMontage))
		{
			BaseCharacter->StopAnimMontage(EquipAnimMontage);
		}

		CurrentRangedWeapon->StopFire();
		CurrentRangedWeapon->EndReload(false);

		CurrentRangedWeapon->SetCurrentWeaponMode(CurrentRangedWeapon->GetCurrentWeaponModeIndex() + 1);

		if (CurrentRangedWeapon->GetCurrentAmmo() < 1 && CanReloadCurrentWeapon())
		{
			CurrentRangedWeapon->StartAutoReload();
		}
		else
		{
			OnCurrentWeaponAmmoChanged(CurrentRangedWeapon->GetCurrentAmmo());
		}
	}
}

bool UCharacterEquipmentComponent::CanReloadCurrentWeapon()
{
	return CurrentRangedWeapon.IsValid() && !CurrentRangedWeapon->IsReloading() && !IsCurrentWeaponMagazineFull() && GetAvailableAmmoForWeaponMagazine(CurrentRangedWeapon.Get()) > 0;
}

void UCharacterEquipmentComponent::OnWeaponMagazineEmpty()
{
	if (CurrentRangedWeapon.IsValid() && CanReloadCurrentWeapon())
	{
		CurrentRangedWeapon->StartAutoReload();
	}
	else
	{
		CurrentRangedWeapon->StopFire();
	}
}

void UCharacterEquipmentComponent::LoadWeaponMagazineByBullet(ARangedWeaponItem* RangedWeaponItem)
{
	int32 AmmoToLoad = 0;
	for (int32 Bullet = 0; Bullet < RangedWeaponItem->GetMagazineSize(); Bullet++)
	{
		const int32 AvailableAmmo = GetAvailableAmmoForWeaponMagazine(RangedWeaponItem);
		if (AvailableAmmo == 0)
		{
			break;
		}
		AmmoToLoad += AvailableAmmo;
	}

	AmmoToLoad = RemoveAmmo(RangedWeaponItem->GetAmmoType(), AmmoToLoad);
	if (AmmoToLoad)
	{
		RangedWeaponItem->SetCurrentAmmo(AmmoToLoad);
	}
}

void UCharacterEquipmentComponent::OnRep_EquipmentAmmoArray()
{
	UpdateAmmoHUDWidgets();
}

void UCharacterEquipmentComponent::UpdateAmmoHUDWidgets()
{
	if (CurrentRangedWeapon.IsValid())
	{
		OnCurrentWeaponAmmoChanged(CurrentRangedWeapon->GetCurrentAmmo());
	}
	else
	{
		OnCurrentWeaponAmmoChanged(0);
	}

	if (CurrentThrowableItem.IsValid())
	{
		OnCurrentThrowableAmmoChanged(EquipmentAmmoArray[(int32)CurrentThrowableItem->GetAmmoType()]);
	}
	else
	{
		// Display the primary item ammo even if no throwables are equipped
		AThrowableItem* PrimaryItem = Cast<AThrowableItem>(EquippedItemsArray[(uint32)EEquipmentItemSlot::PrimaryItem]);
		if (IsValid(PrimaryItem))
		{
			OnCurrentThrowableAmmoChanged(EquipmentAmmoArray[(int32)PrimaryItem->GetAmmoType()]);
		}
		else
		{
			OnCurrentThrowableAmmoChanged(0);
		}
	}
}

void UCharacterEquipmentComponent::CreateLoadout()
{
	if (!IsValid(BaseCharacter) || BaseCharacter->GetRemoteRole() == ROLE_Authority)
	{
		return;
	}

	USkeletalMeshComponent* SkeletalMesh = BaseCharacter->GetMesh();
	if (!IsValid(SkeletalMesh))
	{
		return;
	}

	for (const TPair<EWeaponAmmoType, int32>& AmmoPair : MaxEquippedWeaponAmmo)
	{
		AddAmmo(AmmoPair.Key, AmmoPair.Value);
	}

	for (const TPair<EEquipmentItemSlot, TSubclassOf<AEquipmentItem>>& SlotPair : EquipmentSlots)
	{
		if (!IsValid(SlotPair.Value))
		{
			continue;
		}

		LoadoutOneItem(SlotPair.Key, SlotPair.Value, SkeletalMesh);
	}

	EquipFromDefaultItemSlot();
	UpdateAmmoHUDWidgets();
}

void UCharacterEquipmentComponent::LoadoutOneItem(EEquipmentItemSlot EquipmentSlot, TSubclassOf<AEquipmentItem> EquipmentItemClass, USkeletalMeshComponent* SkeletalMesh, int32 CountInSlot/* = -1*/)
{
	if (CountInSlot == 0 || CountInSlot < -1)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = BaseCharacter;
	AEquipmentItem* EquipmentItem = GetWorld()->SpawnActor<AEquipmentItem>(EquipmentItemClass, SpawnParameters);
	EquipmentItem->AttachToComponent(SkeletalMesh, FAttachmentTransformRules::KeepRelativeTransform, EquipmentItem->GetUnequippedSocketName());

	InitializeInventoryItem(EquipmentItem);
	auto InventoryItem = EquipmentItem->GetLinkedInventoryItem();
	if (InventoryItem.IsValid() && InventoryItem->CanStackItems())
	{
		// Only applies to equipment items that serve as ammo units, e.g. grenades
		// Filling the slot using ammo from EquipmentAmmoArray
		// Adding the remaining ammo to the inventory as items

		if (CountInSlot != -1) // -1 indicates loadout on BeginPlay
		{
			AddAmmo(EquipmentItem->GetAmmoType(), CountInSlot);
		}

		InventoryItem->SetCount(0);
		int32 AmmoToLoad = EquipmentAmmoArray[(uint32)EquipmentItem->GetAmmoType()];
		AmmoToLoad -= InventoryItem->AddCount(AmmoToLoad);
		SetAmmo(EquipmentItem->GetAmmoType(), InventoryItem->GetCount());
		BaseCharacter->PickupItem(InventoryItem->GetInventoryItemType(), AmmoToLoad);
	}
	else
	{
		ARangedWeaponItem* RangedWeaponItem = Cast<ARangedWeaponItem>(EquipmentItem);
		if (IsValid(RangedWeaponItem))
		{
			const TArray<FWeaponModeParameters>* WeaponModesArray = RangedWeaponItem->GetWeaponModesArray();
			for (int i = 0; i < WeaponModesArray->Num(); ++i)
			{
				RangedWeaponItem->SetCurrentWeaponMode(i);

				const FWeaponModeParameters* WeaponModeParameters = RangedWeaponItem->GetWeaponModeParameters(i);
				if (WeaponModeParameters && WeaponModeParameters->ReloadType == EWeaponReloadType::ByBullet)
				{
					LoadWeaponMagazineByBullet(RangedWeaponItem);
				}
				else
				{
					int32 AmmoToLoad = GetAvailableAmmoForWeaponMagazine(RangedWeaponItem);
					AmmoToLoad = RemoveAmmo(RangedWeaponItem->GetAmmoType(), AmmoToLoad);
					if (AmmoToLoad)
					{
						RangedWeaponItem->SetCurrentAmmo(AmmoToLoad);
					}
				}
			}
			RangedWeaponItem->SetCurrentWeaponMode(RangedWeaponItem->GetDefaultWeaponModeIndex());
		}
	}

	AThrowableItem* ThrowableItem = Cast<AThrowableItem>(EquipmentItem);
	if (IsValid(ThrowableItem) && !CanThrowItem(ThrowableItem))
	{
		ThrowableItem->SetActorHiddenInGame(true);
	}

	EquippedItemsArray[(int32)EquipmentSlot] = EquipmentItem;
}

bool UCharacterEquipmentComponent::AddEquipmentItem(TSubclassOf<AEquipmentItem> EquipmentItemClass, int32 Amount/* = 1*/, int32 EquipmentSlotIndex/* = -1*/)
{
	if (Amount < 1 || EquipmentSlotIndex == 0 || EquipmentSlotIndex < -1 || !IsValid(EquipmentItemClass) || !IsValid(BaseCharacter))
	{
		return false;
	}

	AEquipmentItem* DefaultItem = Cast<AEquipmentItem>(EquipmentItemClass->GetDefaultObject());
	if (!IsValid(DefaultItem))
	{
		return false;
	}

	EEquipmentItemSlot EquipmentSlot = (EEquipmentItemSlot)EquipmentSlotIndex;
	if (EquipmentSlotIndex == -1) // indicates the request to find a compatible slot
	{
		EquipmentSlot = FindCompatibleSlot(DefaultItem);
	}

	if (!DefaultItem->IsEquipmentSlotCompatible(EquipmentSlot))
	{
		return false;
	}

	// If an item is already equipped
	AEquipmentItem* EquippedItem = EquippedItemsArray[(int32)EquipmentSlot];
	if (IsValid(EquippedItem))
	{
		const TWeakObjectPtr<UInventoryItem> InventoryItem = EquippedItem->GetLinkedInventoryItem();
		if (!InventoryItem.IsValid())
		{
			return false;
		}

		if (EquippedItem->GetEquipmentItemType() == DefaultItem->GetEquipmentItemType())
		{
			if (InventoryItem->CanStackItems() && InventoryItem->GetAvailableSpaceInStack() > 0)
			{
				const int32 PreviousCount = InventoryItem->GetCount();
				const int32 Remainder = Amount - InventoryItem->AddCount(Amount);
				if (Remainder && !BaseCharacter->PickupItem(InventoryItem->GetInventoryItemType(), Remainder))
				{
					InventoryItem->SetCount(PreviousCount);
					return false;
				}
				SetAmmo(EquippedItem->GetAmmoType(), InventoryItem->GetCount());

				if (IsValid(EquipmentViewWidget))
				{
					EquipmentViewWidget->UpdateSlot((int32)EquipmentSlot);
				}

				return true;
			}

			return false;
		}

		if (!InventoryItem->RemoveFromEquipment(BaseCharacter, (int32)EquipmentSlot))
		{
			return false;
		}
	}

	USkeletalMeshComponent* SkeletalMesh = BaseCharacter->GetMesh();
	if (!IsValid(SkeletalMesh))
	{
		return false;
	}
	LoadoutOneItem(EquipmentSlot, EquipmentItemClass, SkeletalMesh, Amount);

	if (IsValid(EquipmentViewWidget))
	{
		EquipmentViewWidget->UpdateSlot((int32)EquipmentSlot);
	}

	EquipPreviousItemIfUnequipped(false);

	return true;
}

EEquipmentItemSlot UCharacterEquipmentComponent::FindCompatibleSlot(AEquipmentItem* EquipmentItem)
{
	// Find a free slot or a slot holding an item of different type

	EEquipmentItemSlot EquipmentSlot = EEquipmentItemSlot::None;
	for (const auto& Slot : EquipmentItem->GetCompatibleEquipmentSlots())
	{
		AEquipmentItem* EquippedItem = EquippedItemsArray[(int32)Slot];

		if (IsValid(EquippedItem))
		{
			if (EquippedItem->GetEquipmentItemType() == EquipmentItem->GetEquipmentItemType())
			{
				if (!EquippedItem->GetLinkedInventoryItem().IsValid() || !EquippedItem->GetLinkedInventoryItem()->CanStackItems())
				{
					continue;
				}
			}
			EquipmentSlot = EquipmentSlot == EEquipmentItemSlot::None ? Slot : EquipmentSlot;
		}
		else
		{
			EquipmentSlot = Slot;
			break;
		}
	}

	return EquipmentSlot;
}

void UCharacterEquipmentComponent::OnEquipmentSlotUpdated(int32 SlotIndex)
{
	UpdateAmmoHUDWidgets();
}

bool UCharacterEquipmentComponent::RemoveEquipmentItem(int32 EquipmentSlotIndex)
{
	if (EquipmentSlotIndex > EquippedItemsArray.Num())
	{
		return false;
	}

	AEquipmentItem* EquipmentItem = EquippedItemsArray[EquipmentSlotIndex];
	if (IsValid(EquipmentItem))
	{
		if (CurrentSlotIndex == EquipmentSlotIndex)
		{
			UnequipCurrentItem();
		}

		auto InventoryItem = EquipmentItem->GetLinkedInventoryItem();
		if (InventoryItem.IsValid() && InventoryItem->CanStackItems())
		{
			// Only applies to equipment items that serve as ammo units, e.g. grenades

			SetAmmo(EquipmentItem->GetAmmoType(), 0);
		}

		ARangedWeaponItem* RangedWeapon = Cast<ARangedWeaponItem>(EquipmentItem);
		if (IsValid(RangedWeapon))
		{
			if (!AddAmmo(RangedWeapon->GetAmmoType(), RangedWeapon->GetCurrentAmmo()))
			{
				EquipPreviousItemIfUnequipped(false);
				return false;
			}
		}

		EquipmentItem->Destroy();
		EquippedItemsArray[EquipmentSlotIndex] = nullptr;

		if (IsValid(EquipmentViewWidget))
		{
			EquipmentViewWidget->UpdateSlot(EquipmentSlotIndex);
		}

		return true;
	}

	return false;
}

void UCharacterEquipmentComponent::EquipFromDefaultItemSlot(const bool bShouldSkipAnimation/* = true*/)
{
	if (CurrentEquippedItem.IsValid() && (EEquipmentItemSlot)CurrentSlotIndex == DefaultEquipmentItemSlot)
	{
		return;
	}

	const uint32 SlotIndex = (uint32)DefaultEquipmentItemSlot;
	AEquipmentItem* DefaultItem = EquippedItemsArray[SlotIndex];
	if (IsValid(DefaultItem))
	{
		if (CurrentEquippedItem.IsValid())
		{
			UnequipItem(CurrentEquippedItem.Get());
		}

		CurrentSlotIndex = SlotIndex;
		EquipItem(DefaultItem, bShouldSkipAnimation);
	}
}

void UCharacterEquipmentComponent::DrawNextItem()
{
	if (CurrentEquippedItem.IsValid())
	{
		UnequipItem(CurrentEquippedItem.Get());
	}

	AEquipmentItem* NextItem = GetNextItem();
	if (IsValid(NextItem))
	{
		EquipItem(NextItem);
	}
	else if (CurrentSlotIndex == 0 && BaseCharacter->GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_EquipItemBySlotType((EEquipmentItemSlot)CurrentSlotIndex);
	}
}

void UCharacterEquipmentComponent::DrawPreviousItem()
{
	if (CurrentEquippedItem.IsValid())
	{
		UnequipItem(CurrentEquippedItem.Get());
	}

	AEquipmentItem* PreviousItem = GetPreviousItem();
	if (IsValid(PreviousItem))
	{
		EquipItem(PreviousItem);
	}
	else if (CurrentSlotIndex == 0 && BaseCharacter->GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_EquipItemBySlotType((EEquipmentItemSlot)CurrentSlotIndex);
	}
}

bool UCharacterEquipmentComponent::IncrementCurrentSlotIndex()
{
	CurrentSlotIndex++;
	if (CurrentSlotIndex == EquippedItemsArray.Num())
	{
		CurrentSlotIndex = 0;
		return false;
	}

	if (WeaponSwitchIgnoredSlots.Contains((EEquipmentItemSlot)CurrentSlotIndex))
	{
		if (!IncrementCurrentSlotIndex())
		{
			return false;
		}
	}
	return true;
}

bool UCharacterEquipmentComponent::DecrementCurrentSlotIndex()
{
	CurrentSlotIndex--;
	if (CurrentSlotIndex == 0)
	{
		return false;
	}
	if (CurrentSlotIndex == -1)
	{
		CurrentSlotIndex = EquippedItemsArray.Num() - 1;
	}

	if (WeaponSwitchIgnoredSlots.Contains((EEquipmentItemSlot)CurrentSlotIndex))
	{
		if (!DecrementCurrentSlotIndex())
		{
			return false;
		}
	}
	return true;
}

AEquipmentItem* UCharacterEquipmentComponent::GetNextItem()
{
	if (!IncrementCurrentSlotIndex())
	{
		return nullptr;
	}

	AEquipmentItem* NextItem = EquippedItemsArray[CurrentSlotIndex];
	if (!IsValid(NextItem))
	{
		NextItem = GetNextItem();
	}
	return NextItem;
}

AEquipmentItem* UCharacterEquipmentComponent::GetPreviousItem()
{
	if (!DecrementCurrentSlotIndex())
	{
		return nullptr;
	}

	AEquipmentItem* PreviousItem = EquippedItemsArray[CurrentSlotIndex];
	if (!IsValid(PreviousItem))
	{
		PreviousItem = GetPreviousItem();
	}
	return PreviousItem;
}

void UCharacterEquipmentComponent::UnequipCurrentItem()
{
	UnequipItem(CurrentEquippedItem.Get());
}

void UCharacterEquipmentComponent::EquipPreviousItemIfUnequipped(const bool bShouldSkipAnimation/* = true*/)
{
	if (CurrentEquippedItem.IsValid())
	{
		return;
	}

	AEquipmentItem* PreviousItem = EquippedItemsArray[CurrentSlotIndex];
	if (IsValid(PreviousItem))
	{
		EquipItem(PreviousItem, bShouldSkipAnimation);
	}
}


void UCharacterEquipmentComponent::OnRep_IsPrimaryItemEquipped()
{
	if (bIsPrimaryItemEquipped)
	{
		EquipPrimaryItem(true);
	}
	else
	{
		UnequipPrimaryItem(true);
	}
}

void UCharacterEquipmentComponent::EquipPrimaryItem(const bool bForceEquip/* = false*/)
{
	if (!bForceEquip && bIsPrimaryItemEquipped)
	{
		return;
	}

	AEquipmentItem* PrimaryItem = EquippedItemsArray[(uint32)EEquipmentItemSlot::PrimaryItem];
	if (IsValid(PrimaryItem))
	{
		AThrowableItem* ThrowableItem = Cast<AThrowableItem>(PrimaryItem);
		if (!CanThrowItem(ThrowableItem))
		{
			return;
		}

		PrimaryItem->SetActorHiddenInGame(false);
		if (CurrentEquippedItem.IsValid())
		{
			UnequipItem(CurrentEquippedItem.Get());
		}
		EquipItem(PrimaryItem);
		bIsPrimaryItemEquipped = true;
	}
}

void UCharacterEquipmentComponent::UnequipPrimaryItem(const bool bForceUnequip/* = false*/)
{
	if (!bForceUnequip && !bIsPrimaryItemEquipped)
	{
		return;
	}

	UnequipItem(CurrentEquippedItem.Get());
	bIsPrimaryItemEquipped = false;
	EquipPreviousItemIfUnequipped();
}

void UCharacterEquipmentComponent::UnequipItem(AEquipmentItem* EquipmentItem)
{
	if (!IsValid(EquipmentItem) || !IsValid(BaseCharacter->GetMesh()))
	{
		return;
	}

	UAnimMontage* EquipItemAnimMontage = EquipmentItem->GetEquipItemAnimMontage();
	if (IsValid(EquipItemAnimMontage))
	{
		BaseCharacter->StopAnimMontage(EquipItemAnimMontage);
	}

	ARangedWeaponItem* UnequippedRangedWeapon = Cast<ARangedWeaponItem>(EquipmentItem);
	if (IsValid(UnequippedRangedWeapon))
	{
		UnequippedRangedWeapon->StopFire();
		UnequippedRangedWeapon->EndReload(false);
		UnequippedRangedWeapon->OnAmmoChanged.Remove(OnAmmoChangedDelegate);
		UnequippedRangedWeapon->OnWeaponReloaded.Remove(OnWeaponReloadedDelegate);
		UnequippedRangedWeapon->OnMagazineEmpty.Remove(OnWeaponMagazineEmptyDelegate);
	}
	else
	{
		AThrowableItem* UnequippedThrowableItem = Cast<AThrowableItem>(EquipmentItem);
		if (IsValid(UnequippedThrowableItem))
		{
			UAnimMontage* ThrowItemAnimMontage = CurrentThrowableItem->GetEquipItemAnimMontage();
			if (IsValid(ThrowItemAnimMontage))
			{
				BaseCharacter->StopAnimMontage(ThrowItemAnimMontage);
			}

			UnequippedThrowableItem->OnThrowEndEvent.Remove(OnItemThrownDelegate);
			UnequippedThrowableItem->OnThrowAnimationFinishedEvent.Remove(OnItemThrownAnimationFinishedDelegate);
		}
		else
		{
			AMeleeWeaponItem* UnequippedMeleeWeapon = Cast <AMeleeWeaponItem>(EquipmentItem);
			if (IsValid(UnequippedMeleeWeapon))
			{
				UnequippedMeleeWeapon->OnAttackActivatedEvent.Remove(OnMeleeAttackActivated);
			}
		}
	}

	EquipmentItem->AttachToComponent(BaseCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, EquipmentItem->GetUnequippedSocketName());
	EquipmentItem->SetIsEquipped(false);
	CurrentEquippedItem = nullptr;
	CurrentRangedWeapon = nullptr;
	CurrentThrowableItem = nullptr;
	CurrentMeleeWeapon = nullptr;
	UpdateAmmoHUDWidgets();

	BaseCharacter->SetIsAiming(false);

	if (OnEquipmentItemChangedEvent.IsBound())
	{
		OnEquipmentItemChangedEvent.Broadcast(CurrentEquippedItem.Get());
	}
}

void UCharacterEquipmentComponent::EquipItem(AEquipmentItem* EquipmentItem, const bool bShouldSkipAnimation/* = false*/)
{
	if (!IsValid(EquipmentItem))
	{
		return;
	}
	CurrentEquippedItem = EquipmentItem;

	CurrentRangedWeapon = Cast<ARangedWeaponItem>(CurrentEquippedItem);
	if (CurrentRangedWeapon.IsValid())
	{
		OnAmmoChangedDelegate = CurrentRangedWeapon->OnAmmoChanged.AddUFunction(this, FName("OnCurrentWeaponAmmoChanged"));
		OnWeaponReloadedDelegate = CurrentRangedWeapon->OnWeaponReloaded.AddUFunction(this, FName("OnCurrentWeaponReloaded"));
		OnWeaponMagazineEmptyDelegate = CurrentRangedWeapon->OnMagazineEmpty.AddUFunction(this, FName("OnWeaponMagazineEmpty"));
	}
	else
	{
		CurrentThrowableItem = Cast<AThrowableItem>(CurrentEquippedItem);
		if (CurrentThrowableItem.IsValid())
		{
			OnItemThrownDelegate = CurrentThrowableItem->OnThrowEndEvent.AddUFunction(this, FName("OnThrowItemEnd"));
			OnItemThrownAnimationFinishedDelegate = CurrentThrowableItem->OnThrowAnimationFinishedEvent.AddUFunction(this, FName("OnThrowItemAnimationFinished"));
		}
		else
		{
			CurrentMeleeWeapon = Cast<AMeleeWeaponItem>(CurrentEquippedItem);
			if (CurrentMeleeWeapon.IsValid())
			{
				OnMeleeAttackActivated = CurrentMeleeWeapon->OnAttackActivatedEvent.AddUFunction(this, FName("SetIsMeleeAttackActive"));
			}
		}
	}

	UpdateAmmoHUDWidgets();

	if (OnEquipmentItemChangedEvent.IsBound())
	{
		OnEquipmentItemChangedEvent.Broadcast(CurrentEquippedItem.Get());
	}

	UAnimMontage* EquipItemAnimMontage = CurrentEquippedItem->GetEquipItemAnimMontage();
	if (!bShouldSkipAnimation && IsValid(EquipItemAnimMontage))
	{
		const float Duration = BaseCharacter->PlayAnimMontage(EquipItemAnimMontage);
		GetWorld()->GetTimerManager().SetTimer(EquipItemTimer, this, &UCharacterEquipmentComponent::AttachCurrentEquippedItemToCharacterMesh, Duration, false);
	}
	else
	{
		AttachCurrentEquippedItemToCharacterMesh();
	}

	if (IsValid(BaseCharacter) && BaseCharacter->GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_EquipItemBySlotType((EEquipmentItemSlot)CurrentSlotIndex);
	}
}

void UCharacterEquipmentComponent::AttachCurrentEquippedItemToCharacterMesh()
{
	if (CurrentEquippedItem.IsValid())
	{
		if (!IsValid(BaseCharacter))
		{
			return;
		}

		if (IsValid(BaseCharacter->GetMesh()))
		{
			CurrentEquippedItem->AttachToComponent(BaseCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, CurrentEquippedItem->GetEquippedSocketName());
		}
		CurrentEquippedItem->SetIsEquipped(true);

		if (BaseCharacter->IsLocallyControlled() && CurrentRangedWeapon.IsValid() && CurrentRangedWeapon->GetCurrentAmmo() <= 0 && CanReloadCurrentWeapon())
		{
			CurrentRangedWeapon->StartAutoReload();
		}
	}
}

bool UCharacterEquipmentComponent::CanThrowItem(AThrowableItem* ThrowableItem)
{
	return IsValid(ThrowableItem) && !ThrowableItem->IsThrowing() && EquipmentAmmoArray[(int32)ThrowableItem->GetAmmoType()] > 0;
}

void UCharacterEquipmentComponent::ThrowItem()
{
	if (!CanThrowItem(CurrentThrowableItem.Get()))
	{
		return;
	}

	TSubclassOf<AXyzProjectile> ProjectileClass = CurrentThrowableItem->GetProjectileClass();
	FProjectilePool* ProjectilePool = ProjectilePools.FindByPredicate([ProjectileClass](const FProjectilePool& Pool) { return Pool.ProjectileClass == ProjectileClass; });
	if (!ProjectilePool)
	{
		return;
	}

	AXyzProjectile* ThrowableProjectile = ProjectilePool->GetNextAvailableProjectile();
	if (IsValid(ThrowableProjectile))
	{
		Server_OnThrowItem(ThrowableProjectile, ProjectilePool->PoolWorldLocation);
	}
}

void UCharacterEquipmentComponent::SetAmmo(EWeaponAmmoType AmmoType, int32 NewAmmo)
{
	if (NewAmmo >= 0)
	{
		EquipmentAmmoArray[(uint32)AmmoType] = NewAmmo;
	}
}

bool UCharacterEquipmentComponent::AddAmmo(EWeaponAmmoType AmmoType, int32 Amount)
{
	if (Amount < 1 || AmmoType == EWeaponAmmoType::None)
	{
		return false;
	}

	EquipmentAmmoArray[(uint32)AmmoType] += Amount;
	return true;
}

int32 UCharacterEquipmentComponent::RemoveAmmo(EWeaponAmmoType AmmoType, int32 Amount)
{
	if (Amount < 1 || AmmoType == EWeaponAmmoType::None)
	{
		return 0;
	}

	const uint32 AmmoIndex = (uint32)AmmoType;
	const int32 AmmoToRemove = Amount < (int32)EquipmentAmmoArray[AmmoIndex] ? Amount : EquipmentAmmoArray[AmmoIndex];
	EquipmentAmmoArray[AmmoIndex] -= AmmoToRemove;

	return AmmoToRemove;
}

void UCharacterEquipmentComponent::CreateViewWidget(APlayerController* PlayerController)
{
	if (IsValid(EquipmentViewWidget))
	{
		return;
	}

	if (!IsValid(PlayerController) || !IsValid(EquipmentViewWidgetClass))
	{
		return;
	}

	EquipmentViewWidget = CreateWidget<UEquipmentViewWidget>(PlayerController, EquipmentViewWidgetClass);
	EquipmentViewWidget->InitializeWidget(this);
}

void UCharacterEquipmentComponent::InitializeInventoryItem(AEquipmentItem* EquipmentItem, int32 Count/* = 1*/) const
{
	const UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *InventoryItemDataTable.GetUniqueID().GetAssetPathString());
	if (IsValid(DataTable))
	{
		FString RowID = UEnum::GetDisplayValueAsText<EEquipmentItemType>(EquipmentItem->GetEquipmentItemType()).ToString();
		const FInventoryTableRow* ItemData = DataTable->FindRow<FInventoryTableRow>(FName(RowID), TEXT("Find item data"));

		if (ItemData)
		{
			const TWeakObjectPtr<UInventoryItem> NewItem = NewObject<UInventoryItem>(GetOwner(), ItemData->InventoryItemClass);
			NewItem->InitializeItem(ItemData->InventoryItemDescription);
			EquipmentItem->SetLinkedInventoryItem(NewItem);
			NewItem->SetCount(Count);
		}
	}
}

void UCharacterEquipmentComponent::OpenViewEquipment(APlayerController* PlayerController)
{
	if (!IsValid(EquipmentViewWidget))
	{
		CreateViewWidget(PlayerController);
	}

	if (!EquipmentViewWidget->IsVisible())
	{
		EquipmentViewWidget->AddToViewport();
	}
}

void UCharacterEquipmentComponent::CloseViewEquipment()
{
	if (EquipmentViewWidget->IsVisible())
	{
		EquipmentViewWidget->RemoveFromParent();
	}
}

bool UCharacterEquipmentComponent::IsViewEquipmentVisible() const
{
	if (IsValid(EquipmentViewWidget))
	{
		return EquipmentViewWidget->IsVisible();
	}
	return false;
}

void UCharacterEquipmentComponent::Server_OnThrowItem_Implementation(AXyzProjectile* ThrowableProjectile, const FVector ResetLocation)
{
	Multicast_OnThrowItem(ThrowableProjectile, ResetLocation);
}

void UCharacterEquipmentComponent::Multicast_OnThrowItem_Implementation(AXyzProjectile* ThrowableProjectile, const FVector ResetLocation)
{
	if (CurrentThrowableItem.IsValid())
	{
		CurrentThrowableItem->Throw(ThrowableProjectile, ResetLocation);
	}
}

bool UCharacterEquipmentComponent::EquipItemBySlotType(EEquipmentItemSlot EquipmentItemSlot, const bool bShouldSkipAnimation/* = true*/)
{
	if (!IsValid(BaseCharacter))
	{
		BaseCharacter = StaticCast<AXyzBaseCharacter*>(GetOwner());
	}

	if (BaseCharacter->IsLocallyControlled() && CurrentEquippedItem.IsValid() && (EEquipmentItemSlot)CurrentSlotIndex == EquipmentItemSlot)
	{
		return true;
	}

	const uint32 SlotIndex = (uint32)EquipmentItemSlot;
	AEquipmentItem* EquipmentItem = EquippedItemsArray[SlotIndex];
	if (!IsValid(EquipmentItem))
	{
		return false;
	}

	if (bIsPrimaryItemEquipped)
	{
		BaseCharacter->TogglePrimaryItem();
	}
	if (CurrentEquippedItem.IsValid())
	{
		UnequipItem(CurrentEquippedItem.Get());
	}

	CurrentSlotIndex = SlotIndex;
	EquipItem(EquipmentItem, bShouldSkipAnimation);

	return true;
}

void UCharacterEquipmentComponent::Server_EquipItemBySlotType_Implementation(const EEquipmentItemSlot EquipmentItemSlot)
{
	if ((int32)EquipmentItemSlot == 0 && CurrentEquippedItem.IsValid())
	{
		UnequipItem(CurrentEquippedItem.Get());
		CurrentSlotIndex = 0;
	}
	else if (EquipmentItemSlot == DefaultEquipmentItemSlot)
	{
		EquipItemBySlotType(EquipmentItemSlot, true);
	}
	else
	{
		EquipItemBySlotType(EquipmentItemSlot, false);
	}
}

void UCharacterEquipmentComponent::OnRep_CurrentSlotIndex(const int32 CurrentSlotIndex_Old)
{
	if (CurrentSlotIndex == 0 && CurrentEquippedItem.IsValid())
	{
		UnequipItem(CurrentEquippedItem.Get());
	}
	else if (CurrentSlotIndex == (int32)DefaultEquipmentItemSlot)
	{
		EquipItemBySlotType((EEquipmentItemSlot)CurrentSlotIndex, true);
	}
	else
	{
		EquipItemBySlotType((EEquipmentItemSlot)CurrentSlotIndex, false);
	}
}

void UCharacterEquipmentComponent::OnRep_EquippedItemsArray()
{
	EquipItemBySlotType((EEquipmentItemSlot)CurrentSlotIndex, true);
}

void UCharacterEquipmentComponent::OnThrowItemEnd()
{
	if (!CurrentThrowableItem.IsValid())
	{
		return;
	}

	RemoveAmmo(CurrentThrowableItem->GetAmmoType(), 1);
	OnCurrentThrowableAmmoChanged(EquipmentAmmoArray[(int32)CurrentThrowableItem->GetAmmoType()]);

	if (bIsPrimaryItemEquipped)
	{
		CurrentThrowableItem->AttachToComponent(BaseCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, CurrentThrowableItem->GetUnequippedSocketName());

		if (!CanThrowItem(CurrentThrowableItem.Get()))
		{
			CurrentThrowableItem->SetActorHiddenInGame(true);
		}
	}
}

void UCharacterEquipmentComponent::OnThrowItemAnimationFinished()
{
	if (!bIsPrimaryItemEquipped)
	{
		return;
	}

	UnequipItem(CurrentEquippedItem.Get());
	bIsPrimaryItemEquipped = false;

	EquipPreviousItemIfUnequipped();
}

void UCharacterEquipmentComponent::OnCurrentThrowableAmmoChanged(const int32 NewAmmo) const
{
	if (OnCurrentThrowableAmmoChangedEvent.IsBound())
	{
		OnCurrentThrowableAmmoChangedEvent.Broadcast(NewAmmo);
	}
}

void UCharacterEquipmentComponent::OnCurrentWeaponAmmoChanged(const int32 NewAmmo)
{
	if (!OnCurrentWeaponAmmoChangedEvent.IsBound())
	{
		return;
	}

	if (CurrentRangedWeapon.IsValid())
	{
		const uint32 EquipmentAmmo = EquipmentAmmoArray[(uint32)CurrentRangedWeapon->GetAmmoType()];
		OnCurrentWeaponAmmoChangedEvent.Broadcast(NewAmmo, EquipmentAmmo);
	}
	else
	{
		OnCurrentWeaponAmmoChangedEvent.Broadcast(0, 0);
	}
}

void UCharacterEquipmentComponent::OnCurrentWeaponReloaded()
{
	if (CurrentRangedWeapon.IsValid() && IsValid(BaseCharacter) && BaseCharacter->GetLocalRole() == ROLE_Authority)
	{
		int32 ReloadedAmmo = GetAvailableAmmoForWeaponMagazine(CurrentRangedWeapon.Get());
		ReloadedAmmo = RemoveAmmo(CurrentRangedWeapon->GetAmmoType(), ReloadedAmmo);
		if (ReloadedAmmo)
		{
			CurrentRangedWeapon->SetCurrentAmmo(CurrentRangedWeapon->GetCurrentAmmo() + ReloadedAmmo);
		}
	}
	BaseCharacter->OnWeaponReloaded();
}

int32 UCharacterEquipmentComponent::GetAvailableAmmoForWeaponMagazine(ARangedWeaponItem* RangedWeaponItem)
{
	if (IsValid(RangedWeaponItem))
	{
		const int32 AvailableEquipmentAmmo = EquipmentAmmoArray[(uint32)RangedWeaponItem->GetAmmoType()];

		if (RangedWeaponItem->GetReloadType() == EWeaponReloadType::ByBullet)
		{
			return FMath::Min(1, AvailableEquipmentAmmo);
		}

		const int32 AvailableSpaceInWeaponMagazine = RangedWeaponItem->GetMagazineSize() - RangedWeaponItem->GetCurrentAmmo();
		return FMath::Min(AvailableSpaceInWeaponMagazine, AvailableEquipmentAmmo);
	}
	return 0;
}

void UCharacterEquipmentComponent::TryReloadNextBullet()
{
	if (!CurrentRangedWeapon.IsValid())
	{
		return;
	}

	OnCurrentWeaponReloaded();

	if (IsCurrentWeaponMagazineFull() || EquipmentAmmoArray[(uint32)CurrentRangedWeapon->GetAmmoType()] < 1)
	{
		const FName ReloadEndSectionName = CurrentRangedWeapon->GetReloadEndSectionName();
		JumpToAnimMontageSection(ReloadEndSectionName);
		CurrentRangedWeapon->EndReload(false);
		return;
	}

	const FName ReloadLoopStartSectionName = CurrentRangedWeapon->GetReloadLoopStartSectionName();
	JumpToAnimMontageSection(ReloadLoopStartSectionName);
}

bool UCharacterEquipmentComponent::IsCurrentWeaponMagazineFull() const
{
	if (CurrentRangedWeapon.IsValid() && CurrentRangedWeapon->GetMagazineSize() - CurrentRangedWeapon->GetCurrentAmmo() <= 0)
	{
		return true;
	}
	return false;
}

void UCharacterEquipmentComponent::JumpToAnimMontageSection(const FName ReloadLoopStartSectionName) const
{
	if (!CurrentRangedWeapon.IsValid())
	{
		return;
	}

	if (IsValid(BaseCharacter->GetMesh()))
	{
		UAnimInstance* CharacterAnimInstance = BaseCharacter->GetMesh()->GetAnimInstance();
		if (IsValid(CharacterAnimInstance))
		{
			const UAnimMontage* CharacterReloadAnimMontage = CurrentRangedWeapon->GetCharacterReloadAnimMontage();
			CharacterAnimInstance->Montage_JumpToSection(ReloadLoopStartSectionName, CharacterReloadAnimMontage);
		}
	}

	if (IsValid(CurrentRangedWeapon->GetMesh()))
	{
		UAnimInstance* WeaponAnimInstance = CurrentRangedWeapon->GetMesh()->GetAnimInstance();
		if (IsValid(WeaponAnimInstance))
		{
			const UAnimMontage* WeaponReloadAnimMontage = CurrentRangedWeapon->GetWeaponReloadAnimMontage();
			WeaponAnimInstance->Montage_JumpToSection(ReloadLoopStartSectionName, WeaponReloadAnimMontage);
		}
	}
}
