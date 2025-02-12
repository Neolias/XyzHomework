// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EquipmentSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UInventorySlotWidget;
class AEquipmentItem;
class UInventoryItem;

UCLASS()
class XYZHOMEWORK_API UEquipmentSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DELEGATE_RetVal_ThreeParams(bool, FOnEquipmentDropInSlot, const TSubclassOf<AEquipmentItem>&, int32, int32);
	DECLARE_DELEGATE_RetVal_OneParam(bool, FOnEquipmentRemoveFromSlot, int32);
	DECLARE_DELEGATE_OneParam(FOnEquipmentSlotUpdated, int32)

	FOnEquipmentDropInSlot OnEquipmentDropInSlot;
	FOnEquipmentRemoveFromSlot OnEquipmentRemoveFromSlot;
	FOnEquipmentSlotUpdated OnEquipmentSlotUpdated;

	void InitializeSlot(TWeakObjectPtr<UInventoryItem> InventoryItem, int32 SlotIndex);
	void UpdateView();
	bool SetLinkedSlotItem(TWeakObjectPtr<UInventoryItem> NewItem);

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemName;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SlotName;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UInventorySlotWidget> DragAndDropWidgetClass;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
	TWeakObjectPtr<UInventoryItem> LinkedInventoryItem;
	int32 SlotIndexInComponent = 0;

	bool StackSlotItems(TWeakObjectPtr<UInventoryItem> OtherSlotItem);
	bool SwapSlotItems(TWeakObjectPtr<UInventoryItem> OtherSlotItem);
	bool UpdatePreviousSlot(TWeakObjectPtr<UInventoryItem> SlotReference, TWeakObjectPtr<UInventoryItem> NewSlotItem);
};
