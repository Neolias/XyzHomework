// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactive/InteractiveActor.h"
#include "Zipline.generated.h"

/**
 *
 */
UCLASS(Blueprintable)
class XYZHOMEWORK_API AZipline : public AInteractiveActor
{
	GENERATED_BODY()

public:
	AZipline();
	FVector GetZiplineStartLocation() const { return ZiplineStartLocation; }
	FVector GetZiplineEndLocation() const { return ZiplineEndLocation; }
	FVector GetZiplineSpanVector() const { return ZiplineSpanVector; }
	float GetZiplineLength() const { return ZiplineLength; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zipline Parameters")
	UStaticMeshComponent* LeftPillarMeshComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zipline Parameters")
	UStaticMeshComponent* RightPillarMeshComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zipline Parameters")
	UStaticMeshComponent* ZiplineMeshComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline Parameters", meta = (ClampMin = 0.f, UIMin = 0.f))
	float LeftPillarHeight = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline Parameters", meta = (MakeEditWidget))
	FVector LeftPillarOffset = FVector(0.f, 0.f, 0.f);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline Parameters", meta = (ClampMin = 0.f, UIMin = 0.f))
	float RightPillarHeight = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline Parameters", meta = (MakeEditWidget))
	FVector RightPillarOffset = FVector(-200.f, -200.f, 0.f);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline Parameters", meta = (ClampMin = 0.f, UIMin = 0.f))
	float InteractiveVolumeCapsuleRadius = 90.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline Parameters")
	float InteractiveVolumeEndsExtend = 100.f;

	FVector ZiplineStartLocation = FVector::ZeroVector;
	FVector ZiplineEndLocation = FVector::ZeroVector;
	FVector ZiplineSpanVector = FVector::ZeroVector;
	float ZiplineLength = 0.f;

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void CalculateZiplineGeometry();
};
