// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

#include "Actors/Equipment/Throwables/ThrowableItem.h"
#include "Actors/Equipment/Weapons/MeleeWeaponItem.h"
#include "Characters/XyzBaseCharacter.h"
#include "Actors/Equipment/Weapons/RangedWeaponItem.h"
#include "Net/UnrealNetwork.h"

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
	checkf(GetOwner()->IsA<AXyzBaseCharacter>(), TEXT("UCharacterEquipmentComponent::CreateLoadout() should be used only with AXyzBaseCharacter"))
		BaseCharacter = StaticCast<AXyzBaseCharacter*>(GetOwner());
	
	if (BaseCharacter->GetRemoteRole() != ROLE_Authority)
	{
		InstantiateProjectilePools(BaseCharacter);
		CreateLoadout();
		EquipFromDefaultItemSlot();
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
		return CurrentRangedWeapon->GetItemType();
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
	EquipmentAmmoArray[(uint32)RangedWeaponItem->GetAmmoType()] -= AmmoToLoad;
	RangedWeaponItem->SetCurrentAmmo(AmmoToLoad);
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
	if (CurrentThrowableItem.IsValid())
	{
		OnCurrentThrowableAmmoChanged(EquipmentAmmoArray[(int32)CurrentThrowableItem->GetAmmoType()]);
	}
	else
	{
		const AThrowableItem* PrimaryItem = Cast<AThrowableItem>(EquippedItemsArray[(uint32)EEquipmentItemSlot::PrimaryItem]);
		if (IsValid(PrimaryItem))
		{
			OnCurrentThrowableAmmoChanged(EquipmentAmmoArray[(int32)PrimaryItem->GetAmmoType()]);
		}
	}
}

void UCharacterEquipmentComponent::CreateLoadout()
{
	if (!IsValid(BaseCharacter))
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
		EquipmentAmmoArray[(uint32)AmmoPair.Key] = AmmoPair.Value;
	}

	for (const TPair<EEquipmentItemSlot, TSubclassOf<AEquipmentItem>>& SlotPair : EquipmentSlots)
	{
		if (!IsValid(SlotPair.Value))
		{
			continue;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = BaseCharacter;
		AEquipmentItem* Item = GetWorld()->SpawnActor<AEquipmentItem>(SlotPair.Value, SpawnParameters);
		Item->AttachToComponent(SkeletalMesh, FAttachmentTransformRules::KeepRelativeTransform, Item->GetUnequippedSocketName());

		ARangedWeaponItem* RangedWeaponItem = Cast<ARangedWeaponItem>(Item);
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
					const int32 AmmoToLoad = GetAvailableAmmoForWeaponMagazine(RangedWeaponItem);
					EquipmentAmmoArray[(uint32)RangedWeaponItem->GetAmmoType()] -= AmmoToLoad;
					RangedWeaponItem->SetCurrentAmmo(AmmoToLoad);
				}
			}
			RangedWeaponItem->SetCurrentWeaponMode(RangedWeaponItem->GetDefaultWeaponModeIndex());
		}

		AThrowableItem* ThrowableItem = Cast<AThrowableItem>(Item);
		if (IsValid(ThrowableItem))
		{
			OnCurrentThrowableAmmoChanged(EquipmentAmmoArray[(int32)ThrowableItem->GetAmmoType()]);
			if (!CanThrowItem(ThrowableItem))
			{
				ThrowableItem->SetActorHiddenInGame(true);
			}
		}

		EquippedItemsArray[(uint32)SlotPair.Key] = Item;
	}
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

void UCharacterEquipmentComponent::EquipPreviousItemIfUnequipped()
{
	if (CurrentEquippedItem.IsValid())
	{
		return;
	}

	AEquipmentItem* PreviousItem = EquippedItemsArray[CurrentSlotIndex];
	if (IsValid(PreviousItem))
	{
		EquipItem(PreviousItem, true);
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
		const AThrowableItem* ThrowableItem = Cast<AThrowableItem>(PrimaryItem);
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
	OnCurrentWeaponAmmoChanged();

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

bool UCharacterEquipmentComponent::CanThrowItem(const AThrowableItem* ThrowableItem)
{
	return IsValid(ThrowableItem) && EquipmentAmmoArray[(int32)ThrowableItem->GetAmmoType()] > 0;
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

	EquipmentAmmoArray[(int32)CurrentThrowableItem->GetAmmoType()] -= 1;
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

void UCharacterEquipmentComponent::OnCurrentWeaponAmmoChanged(const int32 AmmoAmount/* = 0.f*/)
{
	if (!OnCurrentWeaponAmmoChangedEvent.IsBound())
	{
		return;
	}

	if (CurrentRangedWeapon.IsValid())
	{
		const uint32 EquipmentAmmo = EquipmentAmmoArray[(uint32)CurrentRangedWeapon->GetAmmoType()];
		OnCurrentWeaponAmmoChangedEvent.Broadcast(AmmoAmount, EquipmentAmmo);
	}
	else
	{
		OnCurrentWeaponAmmoChangedEvent.Broadcast(0.f, 0.f);
	}
}

void UCharacterEquipmentComponent::OnCurrentWeaponReloaded()
{
	if (CurrentRangedWeapon.IsValid() && IsValid(BaseCharacter) && BaseCharacter->GetLocalRole() == ROLE_Authority)
	{
		const int32 ReloadedAmmo = GetAvailableAmmoForWeaponMagazine(CurrentRangedWeapon.Get());
		EquipmentAmmoArray[(uint32)CurrentRangedWeapon->GetAmmoType()] -= ReloadedAmmo;
		CurrentRangedWeapon->SetCurrentAmmo(CurrentRangedWeapon->GetCurrentAmmo() + ReloadedAmmo);
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
