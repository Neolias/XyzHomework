// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/WeaponAmmoWidget.h"

void UWeaponAmmoWidget::OnWeaponAmmoChanged(const int32 NewAmmo, const int32 NewAmmoInEquipment)
{
	WeaponAmmo = NewAmmo;
	WeaponAmmoInEquipment = NewAmmoInEquipment;
}

void UWeaponAmmoWidget::OnThrowableAmmoChanged(const int32 NewAmmo)
{
	ThrowableAmmo = NewAmmo;
}
