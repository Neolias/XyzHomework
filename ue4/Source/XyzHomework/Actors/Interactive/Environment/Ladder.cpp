// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Interactive/Environment/Ladder.h"

#include "XyzHomeworkTypes.h"
#include "Components/BoxComponent.h"

ALadder::ALadder()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>("Root");
	InteractiveVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractiveVolume"));
	InteractiveVolume->SetGenerateOverlapEvents(true);
	InteractiveVolume->SetupAttachment(RootComponent);
	TopInteractiveVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TopInteractiveVolume"));
	TopInteractiveVolume->SetGenerateOverlapEvents(true);
	TopInteractiveVolume->SetupAttachment(RootComponent);
	LeftRailMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftRail"));
	LeftRailMeshComponent->SetupAttachment(RootComponent);
	RightRailMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightRail"));
	RightRailMeshComponent->SetupAttachment(RootComponent);
	StepsMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>("Steps");
	StepsMeshComponent->SetupAttachment(RootComponent);
}

FVector ALadder::GetAttachFromTopAnimMontageStartLocation() const
{
	const FRotator OrientationRotation = GetActorForwardVector().ToOrientationRotator();
	const FVector Offset = OrientationRotation.RotateVector(AttachFromTopAnimMontageStartOffset);
	const FVector LadderTop = GetActorLocation() + GetActorUpVector() * LadderHeight;
	return LadderTop + Offset;
}

void ALadder::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	UBoxComponent* TopInteractiveVolumeBounds = StaticCast<UBoxComponent*>(TopInteractiveVolume);
	if (IsValid(TopInteractiveVolumeBounds))
	{
		TopInteractiveVolumeBounds->SetBoxExtent(FVector(TopInteractiveVolumeDepth / 2, LadderWidth / 2, TopInteractiveVolumeHeight / 2));
		TopInteractiveVolumeBounds->SetRelativeLocation(FVector(-TopInteractiveVolumeDepth / 2, 0.f, LadderHeight + TopInteractiveVolumeHeight / 2));
	}

	UBoxComponent* BottomInteractiveVolumeBounds = StaticCast<UBoxComponent*>(InteractiveVolume);
	if (IsValid(BottomInteractiveVolumeBounds))
	{
		BottomInteractiveVolumeBounds->SetBoxExtent(FVector(BottomInteractiveVolumeDepth / 2, LadderWidth / 2, LadderHeight / 2));
		BottomInteractiveVolumeBounds->SetRelativeLocation(FVector(BottomInteractiveVolumeDepth / 2, 0.f, LadderHeight / 2));
	}

	const UStaticMesh* LeftRailMesh = LeftRailMeshComponent->GetStaticMesh();
	if (IsValid(LeftRailMesh))
	{
		const float RailMeshHeight = LeftRailMesh->GetBoundingBox().GetSize().Z;
		if (!FMath::IsNearlyZero(RailMeshHeight))
		{
			LeftRailMeshComponent->SetRelativeScale3D(FVector(RailRadiusScale, RailRadiusScale, LadderHeight / RailMeshHeight));
		}
	}

	const UStaticMesh* RightRailMesh = RightRailMeshComponent->GetStaticMesh();
	if (IsValid(RightRailMesh))
	{
		const float RailMeshHeight = RightRailMesh->GetBoundingBox().GetSize().Z;
		if (!FMath::IsNearlyZero(RailMeshHeight))
		{
			RightRailMeshComponent->SetRelativeScale3D(FVector(RailRadiusScale, RailRadiusScale, LadderHeight / RailMeshHeight));
		}
	}

	const UStaticMesh* StepsMesh = StepsMeshComponent->GetStaticMesh();
	if (IsValid(StepsMesh))
	{
		const float StepMeshWidth = StepsMesh->GetBoundingBox().GetSize().X;
		if (!FMath::IsNearlyZero(StepMeshWidth))
		{
			StepsMeshComponent->SetRelativeScale3D(FVector(LadderWidth / StepMeshWidth, 1.f, 1.f));
		}
	}

	LeftRailMeshComponent->SetRelativeLocation(FVector(0.f, -LadderWidth / 2, LadderHeight / 2));
	RightRailMeshComponent->SetRelativeLocation(FVector(0.f, LadderWidth / 2, LadderHeight / 2));
	StepsMeshComponent->SetRelativeRotation(FVector::RightVector.ToOrientationRotator());

	StepsMeshComponent->ClearInstances();

	const int32 NumOfSteps = FMath::FloorToInt((LadderHeight - FirstStepOffset - LastStepOffset) / StepHeight) + 1;
	for (int32 i = 0; i < NumOfSteps; i++)
	{
		StepsMeshComponent->AddInstance(FTransform(FVector(0.f, 0.f, FirstStepOffset + StepHeight * i)));
	}
}

void ALadder::BeginPlay()
{
	Super::BeginPlay();

	InteractiveVolume->SetCollisionProfileName(CollisionProfilePawnInteractiveVolume);
	TopInteractiveVolume->SetCollisionProfileName(CollisionProfilePawnInteractiveVolume);

	TopInteractiveVolume->OnComponentBeginOverlap.AddDynamic(this, &ALadder::OnInteractiveVolumeBeginOverlap);
	TopInteractiveVolume->OnComponentEndOverlap.AddDynamic(this, &ALadder::OnInteractiveVolumeEndOverlap);
}

void ALadder::OnInteractiveVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, const int32 OtherBodyIndex, const bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnInteractiveVolumeBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (OverlappedComponent == TopInteractiveVolume)
	{
		bIsOnTop = true;
	}
}

void ALadder::OnInteractiveVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, const int32 OtherBodyIndex)
{
	Super::OnInteractiveVolumeEndOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

	if (OverlappedComponent == TopInteractiveVolume)
	{
		bIsOnTop = false;
	}
}
