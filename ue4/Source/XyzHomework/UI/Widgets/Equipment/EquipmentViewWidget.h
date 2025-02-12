// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EquipmentViewWidget.generated.h"

class UGridPanel;
class UCharacterEquipmentComponent;
class UEquipmentSlotWidget;
class AEquipmentItem;

UCLASS()
class XYZHOMEWORK_API UEquipmentViewWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeWidget(UCharacterEquipmentComponent* EquipmentComponent);
	void UpdateSlot(int32 SlotIndex);
	UEquipmentSlotWidget* GetEquipmentSlotWidget(int32 SlotIndex) const;

protected:
	void AddSlotToView(AEquipmentItem* EquipmentItem, int32 SlotIndex);

	bool EquipItem(const TSubclassOf<AEquipmentItem>& WeaponClass, int32 Amount, int32 SenderIndex);
	bool UnequipItem(int32 SlotIndex);
	void UpdateEquipment(int32 SlotIndex);

	UPROPERTY(meta = (BindWidget))
	UGridPanel* ItemSlots;

	UPROPERTY(EditDefaultsOnly, Category = "ItemContainer View Settings")
	TSubclassOf<UEquipmentSlotWidget> DefaultSlotViewClass;

	TWeakObjectPtr<UCharacterEquipmentComponent> CachedEquipmentComponent;
};
