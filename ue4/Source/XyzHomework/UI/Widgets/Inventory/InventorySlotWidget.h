// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySlotWidget.generated.h"

class UTextBlock;
class UInventoryItem;
struct FInventorySlot;
class UImage;

UCLASS()
class XYZHOMEWORK_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeSlot(FInventorySlot& InventorySlot);
	void UpdateView();
	void UpdateItemIconAndCount(TWeakObjectPtr<UInventoryItem> NewItemData);
	void SetItemIcon(UTexture2D* Icon);
	void SetLinkedSlotItem(TWeakObjectPtr<UInventoryItem> NewItem);
	FInventorySlot* GetLinkedSlot() const { return LinkedSlot; }

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemCount;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
	FInventorySlot* LinkedSlot;

	bool StackSlotItems(TWeakObjectPtr<UInventoryItem> OtherSlotItem);
	bool SwapSlotItems(TWeakObjectPtr<UInventoryItem> OtherSlotItem);
	bool UpdatePreviousSlot(TWeakObjectPtr<UInventoryItem> SlotReference, TWeakObjectPtr<UInventoryItem> NewSlotData);

};
