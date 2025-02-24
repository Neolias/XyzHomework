// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <memory>
#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Blueprint/UserWidget.h"
#include "RadialMenuWidget.generated.h"

class UCanvasPanel;
class UScaleBox;
class UCharacterEquipmentComponent;
class UTextBlock;
class UImage;

struct FMenuSegment
{
	float LeftEdgeAngle = 0.f;
	float RightEdgeAngle = 0.f;
	int32 MenuSegmentIndex = -1;
	TArray<std::shared_ptr<FMenuSegment>> EnclosedSegments;
};

UCLASS()
class XYZHOMEWORK_API URadialMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeWidget(UCharacterEquipmentComponent* EquipmentComponent);
	void UpdateMenuSegmentWidgets();

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* Canvas;
	UPROPERTY(meta = (BindWidget))
	UScaleBox* RadialMenu;
	UPROPERTY(meta = (BindWidget))
	UImage* RadialBackground;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* EquippedItemText;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RadialMenu")
	FName SegmentCountMaterialParamName = FName("Segments");
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RadialMenu")
	FName SegmentIndexMaterialParamName = FName("Index");
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RadialMenu")
	TArray<EEquipmentItemSlot> ItemSlotsInMenuSegments;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RadialMenu")
	TSubclassOf<UUserWidget> MenuSegmentWidgetClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RadialMenu")
	FName SegmentItemIconName = FName("ItemIcon");

private:
	void CreateMenuSegmentWidgets();
	void ConfirmSegmentSelection();
	int32 GetSelectedMenuSegmentIndex(FVector2D WidgetAbsolutePosition, FVector2D MouseScreenPosition) const;
	void OnNewSegmentSelected(int32 NewSegmentIndex);

	int32 CurrentSegmentIndex = -1;
	TArray<UUserWidget*> MenuSegmentWidgets;
	std::shared_ptr<FMenuSegment> MenuSegments;
	UPROPERTY()
	UMaterialInstanceDynamic* BackgroundMaterial;
	TWeakObjectPtr<UCharacterEquipmentComponent> CachedEquipmentComponent;
	FVector2D UpVector = FVector2D(0.f, -1.f);
	// How far the mouse cursor should move to trigger recalculation
	float MinMouseDeltaSquared = .1f;

	//@ GRAPH SECTION START

	void GenerateMenuSegmentGraph();
	void AddChildNodes(const std::shared_ptr<FMenuSegment>& ParentNode, float ArcAngle, uint32 Depth = 0);
	void AddLeafNode(const std::shared_ptr<FMenuSegment>& ParentNode, const std::shared_ptr<FMenuSegment>& LeafNode, uint32 Depth = 0);
	std::shared_ptr<FMenuSegment> FindNode(const std::shared_ptr<FMenuSegment>& ParentNode, float RelativeMousePositionAngle, uint32 Depth = 0) const;

	// Radial menu circumference (360 degrees by default)
	float RadialMenuArcAngle = 2 * PI;
	// The menu segment at index 0 is positioned in the middle using this offset
	// Dynamically calculated in NativeConstruct
	float RadialMenuOffsetAngle = 0.f;
	// Every node area is divided to this number of subareas
	// 2 indicates the log(n) graph complexity
	uint32 GraphNodeDivisions = 2;
	// Dynamically calculated in NativeConstruct
	uint32 MaxGraphDepth = 1;

	//~ GRAPH SECTION END

};
