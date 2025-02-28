// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Equipment/RadialMenuWidget.h"

#include "Actors/Equipment/EquipmentItem.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/ScaleBox.h"
#include "Components/TextBlock.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

void URadialMenuWidget::InitializeWidget(UCharacterEquipmentComponent* EquipmentComponent)
{
	CachedEquipmentComponent = EquipmentComponent;
}

void URadialMenuWidget::UpdateMenuSegmentWidgets()
{
	TArray<AEquipmentItem*> EquippedItems;
	if (CachedEquipmentComponent.IsValid())
	{
		EquippedItems = CachedEquipmentComponent->GetEquippedItems();
	}

	if (EquippedItems.Num() == 0)
	{
		return;
	}

	for (int32 i = 0; i < ItemSlotsInMenuSegments.Num(); ++i)
	{
		UImage* ImageIcon = MenuSegmentWidgets[i]->WidgetTree->FindWidget<UImage>(SegmentItemIconName);
		if (IsValid(ImageIcon))
		{
			AEquipmentItem* EquipmentItem = EquippedItems[(int32)ItemSlotsInMenuSegments[i]];

			if (IsValid(EquipmentItem) && EquipmentItem->GetLinkedInventoryItem().IsValid())
			{
				ImageIcon->SetBrushFromTexture(EquipmentItem->GetLinkedInventoryItem()->GetItemDescription().Icon);
			}
			else
			{
				ImageIcon->SetBrushFromTexture(nullptr);
			}
		}
	}
}

void URadialMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ItemSlotsInMenuSegments.Num() > 0)
	{
		RadialMenuOffsetAngle = -PI / ItemSlotsInMenuSegments.Num();
	}

	if (IsValid(RadialBackground))
	{
		RadialBackground->SetRenderTransformAngle(FMath::RadiansToDegrees(RadialMenuOffsetAngle));
		if (!IsValid(BackgroundMaterial))
		{
			BackgroundMaterial = RadialBackground->GetDynamicMaterial();
		}
		BackgroundMaterial->SetScalarParameterValue(SegmentCountMaterialParamName, ItemSlotsInMenuSegments.Num());
	}

	CreateMenuSegmentWidgets();
	GenerateMenuSegmentGraph();
	OnNewSegmentSelected(0);
}

FReply URadialMenuWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (IsVisible() && InMouseEvent.GetCursorDelta().SizeSquared() > MinMouseDeltaSquared)
	{
		const FVector2D WidgetCenterPosition = InGeometry.GetAbsolutePosition() + InGeometry.GetAbsoluteSize() * .5f;
		const int32 NewSegmentIndex = GetSelectedMenuSegmentIndex(WidgetCenterPosition, InMouseEvent.GetScreenSpacePosition());
		OnNewSegmentSelected(NewSegmentIndex);
	}

	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

void URadialMenuWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (IsVisible())
	{
		const FVector2D WidgetCenterPosition = InGeometry.GetAbsolutePosition() + InGeometry.GetAbsoluteSize() * .5f;
		const int32 NewSegmentIndex = GetSelectedMenuSegmentIndex(WidgetCenterPosition, InMouseEvent.GetScreenSpacePosition());
		OnNewSegmentSelected(NewSegmentIndex);
	}
}

FReply URadialMenuWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	const FKey MouseBtn = InMouseEvent.GetEffectingButton();
	if (MouseBtn == EKeys::LeftMouseButton)
	{
		ConfirmSegmentSelection();

		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void URadialMenuWidget::CreateMenuSegmentWidgets()
{
	if (!IsValid(Canvas) || !IsValid(RadialMenu) || !IsValid(MenuSegmentWidgetClass))
	{
		return;
	}

	const UCanvasPanelSlot* RadialMenuSlot = Cast<UCanvasPanelSlot>(RadialMenu->Slot);
	if (!IsValid(RadialMenuSlot))
	{
		return;
	}

	const int32 MenuSegmentsNum = ItemSlotsInMenuSegments.Num();
	MenuSegmentWidgets.Reserve(MenuSegmentsNum);

	const float AngleStep = FMath::RadiansToDegrees(RadialMenuArcAngle / MenuSegmentsNum);
	for (int32 i = 0; i < MenuSegmentsNum; ++i)
	{
		UUserWidget* MenuSegmentWidget = CreateWidgetInstance(*this, MenuSegmentWidgetClass, FName(FString::Printf(TEXT("MenuSegmentImage%i"), i)));
		if (!IsValid(MenuSegmentWidget))
		{
			continue;
		}
		Canvas->AddChildToCanvas(MenuSegmentWidget);

		const float PositionAngle = AngleStep * i + RadialMenuOffsetAngle;
		UCanvasPanelSlot* SegmentImageSlot = Cast<UCanvasPanelSlot>(MenuSegmentWidget->Slot);
		if (!IsValid(SegmentImageSlot))
		{
			Canvas->RemoveChild(MenuSegmentWidget);
			continue;
		}
		SegmentImageSlot->SetAutoSize(true);
		SegmentImageSlot->SetAnchors(FAnchors(.5f, .5f, .5f, .5f));
		SegmentImageSlot->SetAlignment(FVector2D(.5f, .5f));
		const FVector2D NewImagePosition = SegmentImageSlot->GetPosition() + UpVector.GetRotated(PositionAngle) * (RadialMenuSlot->GetSize().Y * .35f);
		SegmentImageSlot->SetPosition(NewImagePosition);

		MenuSegmentWidgets.Add(MenuSegmentWidget);
	}

	UpdateMenuSegmentWidgets();
}

void URadialMenuWidget::ConfirmSegmentSelection()
{
	if (CurrentSegmentIndex >= 0 && CurrentSegmentIndex < ItemSlotsInMenuSegments.Num())
	{
		const EEquipmentItemSlot SelectedSlot = ItemSlotsInMenuSegments[CurrentSegmentIndex];
		CachedEquipmentComponent->EquipItemBySlotType(SelectedSlot, false);
	}
}

int32 URadialMenuWidget::GetSelectedMenuSegmentIndex(FVector2D WidgetAbsolutePosition, FVector2D MouseScreenPosition) const
{
	FVector2D RelativeMousePositionDirection = MouseScreenPosition - WidgetAbsolutePosition;
	RelativeMousePositionDirection.Normalize();
	const int32 DotSign = RelativeMousePositionDirection.X >= 0 ? 1 : -1; // Converting from 0..PI range to 0..2PI range
	const float AngleOffset = RelativeMousePositionDirection.X >= 0.f ? 0.f : PI; // Converting from 0..PI range to 0..2PI range
	float RelativeMousePositionAngle = AngleOffset + FMath::Acos(DotSign * FVector2D::DotProduct(UpVector, RelativeMousePositionDirection)) - RadialMenuOffsetAngle;
	RelativeMousePositionAngle = RelativeMousePositionAngle > 2 * PI ? RelativeMousePositionAngle - 2 * PI : RelativeMousePositionAngle;
	const auto SelectedSegment = FindNode(MenuSegments, RelativeMousePositionAngle);
	if (SelectedSegment)
	{
		return SelectedSegment->MenuSegmentIndex;
	}

	return -1;
}

void URadialMenuWidget::OnNewSegmentSelected(int32 NewSegmentIndex)
{
	if (NewSegmentIndex == CurrentSegmentIndex)
	{
		return;
	}

	CurrentSegmentIndex = NewSegmentIndex;
	BackgroundMaterial->SetScalarParameterValue(SegmentIndexMaterialParamName, NewSegmentIndex);

	if (!IsValid(EquippedItemText) || ItemSlotsInMenuSegments.Num() == 0 || !CachedEquipmentComponent.IsValid()
		|| CachedEquipmentComponent->GetEquippedItems().Num() == 0)
	{
		return;
	}

	AEquipmentItem* EquipmentItem = CachedEquipmentComponent->GetEquippedItems()[(int32)ItemSlotsInMenuSegments[CurrentSegmentIndex]];
	if (IsValid(EquipmentItem) && EquipmentItem->GetLinkedInventoryItem().IsValid())
	{
		EquippedItemText->SetText(EquipmentItem->GetLinkedInventoryItem()->GetItemDescription().Name);
	}
	else
	{
		EquippedItemText->SetText(FText());
	}
}

//@ GRAPH SECTION START

void URadialMenuWidget::GenerateMenuSegmentGraph()
{
	MaxGraphDepth = FMath::Log2(ItemSlotsInMenuSegments.Num()) - 1; // decreased by one to have at least 2 segments in one node
	MenuSegments = std::make_shared<FMenuSegment>();
	MenuSegments->RightEdgeAngle = RadialMenuArcAngle;
	AddChildNodes(MenuSegments, RadialMenuArcAngle);

	const float AngleStep = RadialMenuArcAngle / ItemSlotsInMenuSegments.Num();
	for (int32 i = 0; i < ItemSlotsInMenuSegments.Num(); ++i)
	{
		std::shared_ptr<FMenuSegment> NewSegment = std::make_shared<FMenuSegment>();
		NewSegment->LeftEdgeAngle = AngleStep * i;
		NewSegment->RightEdgeAngle = AngleStep * (i + 1);
		NewSegment->MenuSegmentIndex = i;
		AddLeafNode(MenuSegments, NewSegment);
	}
}

void URadialMenuWidget::AddChildNodes(const std::shared_ptr<FMenuSegment>& ParentNode, float ArcAngle, uint32 Depth/* = 0*/)
{
	if (Depth >= MaxGraphDepth)
	{
		return;
	}

	const float RotationAngle = ArcAngle / GraphNodeDivisions;
	for (uint32 i = 0; i < GraphNodeDivisions; ++i)
	{
		std::shared_ptr<FMenuSegment> NewSegment = std::make_shared<FMenuSegment>();
		NewSegment->LeftEdgeAngle = ParentNode->LeftEdgeAngle + RotationAngle * i;
		NewSegment->RightEdgeAngle = ParentNode->LeftEdgeAngle + RotationAngle * (i + 1);
		AddChildNodes(NewSegment, RotationAngle, Depth + 1);
		ParentNode->EnclosedSegments.Add(NewSegment);
	}
}

void URadialMenuWidget::AddLeafNode(const std::shared_ptr<FMenuSegment>& ParentNode, const std::shared_ptr<FMenuSegment>& LeafNode, uint32 Depth/* = 0*/)
{
	if ((LeafNode->LeftEdgeAngle >= ParentNode->LeftEdgeAngle && LeafNode->LeftEdgeAngle < ParentNode->RightEdgeAngle)
		|| (LeafNode->RightEdgeAngle > ParentNode->LeftEdgeAngle && LeafNode->RightEdgeAngle <= ParentNode->RightEdgeAngle))
	{
		if (Depth >= MaxGraphDepth)
		{
			ParentNode->EnclosedSegments.Add(LeafNode);
			return;
		}

		for (const auto& ChildNode : ParentNode->EnclosedSegments)
		{
			AddLeafNode(ChildNode, LeafNode, Depth + 1);
		}
	}
}

std::shared_ptr<FMenuSegment> URadialMenuWidget::FindNode(const std::shared_ptr<FMenuSegment>& ParentNode, float RelativeMousePositionAngle, uint32 Depth/* = 0*/) const
{
	if (RelativeMousePositionAngle >= ParentNode->LeftEdgeAngle && RelativeMousePositionAngle < ParentNode->RightEdgeAngle)
	{
		if (Depth > MaxGraphDepth)
		{
			return ParentNode;
		}

		for (auto& ChildNode : ParentNode->EnclosedSegments)
		{
			auto Result = FindNode(ChildNode, RelativeMousePositionAngle, Depth + 1);
			if (Result)
			{
				return Result;
			}
		}
	}

	return nullptr;
}

//~ GRAPH SECTION END
