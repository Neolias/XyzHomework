// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactive/Interactable.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Actor.h"
#include "Door.generated.h"

UCLASS()
class XYZHOMEWORK_API ADoor : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	ADoor();
	virtual void Tick(float DeltaTime) override;
	virtual void Interact(APawn* InteractingPawn) override;
	virtual FName GetActionName() override;
	virtual bool HasOnInteractionCallback() override;
	virtual FDelegateHandle AddOnInteractionDelegate(UObject* Object, const FName& FunctionName) override;
	virtual void RemoveOnInteractionDelegate(FDelegateHandle DelegateHandle) override;

protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	void UpdateDoorAnimTimeline(float Alpha) const;
	UFUNCTION()
	void OnDoorAnimFinished();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Interactable"))
	UStaticMeshComponent* DoorMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Interactable"))
	USceneComponent* DoorPivot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Interactable"))
	UCurveFloat* DoorAnimCurve;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Interactable"))
	FVector2D MinMaxAnimAngles = FVector2D(0.f, 120.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Interactable"))
	FName ActionName = FName("InteractWithObject");

	IInteractable::FOnInteraction OnInteraction;

private:
	bool bIsOpen = false;
	UPROPERTY()
	FTimeline DoorAnimTimeline;

};
