// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/XyzBaseCharacter.h"
#include "GameFramework/Actor.h"
#include "InteractiveActor.generated.h"

/**
 *
 */
UCLASS(Abstract, NotBlueprintable)
class XYZHOMEWORK_API AInteractiveActor : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactive Actor Parameters")
	UPrimitiveComponent* InteractiveVolume;

	virtual void BeginPlay() override;
	UFUNCTION()
	virtual void OnInteractiveVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void OnInteractiveVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	virtual bool OverlapsBaseCharacterCapsuleComponent(AXyzBaseCharacter* BaseCharacter, UPrimitiveComponent* Component);
};
