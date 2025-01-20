// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Interactive/Environment/Zipline.h"

#include "XyzHomeworkTypes.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"

AZipline::AZipline()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>("Root");
	InteractiveVolume = CreateDefaultSubobject<UCapsuleComponent>(TEXT("InteractiveVolume"));
	InteractiveVolume->SetGenerateOverlapEvents(true);
	InteractiveVolume->SetupAttachment(RootComponent);
	LeftPillarMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftPillar"));
	LeftPillarMeshComponent->SetupAttachment(RootComponent);
	RightPillarMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightPillar"));
	RightPillarMeshComponent->SetupAttachment(RootComponent);
	ZiplineMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Zipline"));
	ZiplineMeshComponent->SetupAttachment(RootComponent);
}

void AZipline::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	CalculateZiplineGeometry();

	const UStaticMesh* LeftPillarMesh = LeftPillarMeshComponent->GetStaticMesh();
	if (IsValid(LeftPillarMesh))
	{
		const float LeftPillarMeshHeight = LeftPillarMesh->GetBoundingBox().GetSize().Z;
		if (!FMath::IsNearlyZero(LeftPillarMeshHeight))
		{
			LeftPillarMeshComponent->SetRelativeScale3D(FVector(1.f, 1.f, LeftPillarHeight / LeftPillarMeshHeight));
		}
		const FVector Offset = LeftPillarOffset + LeftPillarHeight / 2 * LeftPillarMeshComponent->GetUpVector();
		LeftPillarMeshComponent->SetRelativeLocation(Offset);
	}

	const UStaticMesh* RightPillarMesh = RightPillarMeshComponent->GetStaticMesh();
	if (IsValid(RightPillarMesh))
	{
		const float RightPillarMeshHeight = RightPillarMesh->GetBoundingBox().GetSize().Z;
		if (!FMath::IsNearlyZero(RightPillarMeshHeight))
		{
			RightPillarMeshComponent->SetRelativeScale3D(FVector(1.f, 1.f, RightPillarHeight / RightPillarMeshHeight));
		}
		const FVector Offset = RightPillarOffset + RightPillarHeight / 2 * RightPillarMeshComponent->GetUpVector();
		RightPillarMeshComponent->SetRelativeLocation(Offset);
	}

	const UStaticMesh* ZiplineMesh = ZiplineMeshComponent->GetStaticMesh();
	if (IsValid(ZiplineMesh))
	{
		const float ZiplineMeshLength = ZiplineMesh->GetBoundingBox().GetSize().Y;
		if (!FMath::IsNearlyZero(ZiplineMeshLength))
		{
			ZiplineMeshComponent->SetRelativeScale3D(FVector(1.f, ZiplineLength / ZiplineMeshLength, 1.f));
		}
	}

	ZiplineMeshComponent->SetWorldLocation(ZiplineStartLocation + ZiplineSpanVector / 2);

	FRotator ZiplineRotation = UKismetMathLibrary::FindLookAtRotation(ZiplineMeshComponent->GetForwardVector(), ZiplineSpanVector);
	ZiplineRotation.Pitch += 90.f;
	ZiplineRotation.Roll += 90.f;
	ZiplineMeshComponent->SetRelativeRotation(ZiplineRotation);

	UCapsuleComponent* InteractiveVolumeBounds = StaticCast<UCapsuleComponent*>(InteractiveVolume);
	if (IsValid(InteractiveVolumeBounds))
	{
		InteractiveVolumeBounds->SetCapsuleSize(InteractiveVolumeCapsuleRadius, (ZiplineLength + InteractiveVolumeEndsExtend) / 2);
		InteractiveVolumeBounds->SetWorldLocation(ZiplineStartLocation + ZiplineSpanVector / 2);
		ZiplineRotation.Roll += 90.f;
		InteractiveVolumeBounds->SetRelativeRotation(ZiplineRotation);
	}
}

void AZipline::BeginPlay()
{
	Super::BeginPlay();

	InteractiveVolume->SetCollisionProfileName(CollisionProfilePawnInteractiveVolume);

	CalculateZiplineGeometry();
}

void AZipline::CalculateZiplineGeometry()
{
	const FVector LeftPillarTopLocation = LeftPillarMeshComponent->GetComponentLocation() + LeftPillarHeight / 2 * LeftPillarMeshComponent->GetUpVector();
	const FVector RightPillarTopLocation = RightPillarMeshComponent->GetComponentLocation() + RightPillarHeight / 2 * RightPillarMeshComponent->GetUpVector();

	if (LeftPillarTopLocation.Z > RightPillarTopLocation.Z)
	{
		ZiplineStartLocation = LeftPillarTopLocation;
		ZiplineEndLocation = RightPillarTopLocation;
	}
	else
	{
		ZiplineStartLocation = RightPillarTopLocation;
		ZiplineEndLocation = LeftPillarTopLocation;
	}

	ZiplineLength = FVector::Distance(ZiplineStartLocation, ZiplineEndLocation);
	ZiplineSpanVector = ZiplineEndLocation - ZiplineStartLocation;
}
