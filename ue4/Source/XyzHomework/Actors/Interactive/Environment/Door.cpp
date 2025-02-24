// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Interactive/Environment/Door.h"

ADoor::ADoor()
{
	USceneComponent* DefaultRoot = CreateDefaultSubobject<USceneComponent>(FName(TEXT("DefaultRoot")));
	SetRootComponent(DefaultRoot);

	DoorPivot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorPivot"));
	DoorPivot->SetupAttachment(DefaultRoot);
	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(DoorPivot);

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void ADoor::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(DoorAnimCurve))
	{
		FOnTimelineFloatStatic DoorAnimUpdateDelegate;
		DoorAnimUpdateDelegate.BindUObject(this, &ADoor::UpdateDoorAnimTimeline);
		DoorAnimTimeline.AddInterpFloat(DoorAnimCurve, DoorAnimUpdateDelegate);

		FOnTimelineEventStatic DoorAnimFinishedDelegate;
		DoorAnimFinishedDelegate.BindUObject(this, &ADoor::OnDoorAnimFinished);
		DoorAnimTimeline.SetTimelineFinishedFunc(DoorAnimFinishedDelegate);
	}
}

void ADoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	DoorAnimTimeline.TickTimeline(DeltaTime);
}

void ADoor::Interact(APawn* InteractingPawn)
{
	SetActorTickEnabled(true);
	if (bIsOpen)
	{
		DoorAnimTimeline.Reverse();
	}
	else
	{
		DoorAnimTimeline.Play();
		if (OnInteraction.IsBound())
		{
			OnInteraction.Broadcast();
		}
	}
	bIsOpen = !bIsOpen;
}

FName ADoor::GetActionName()
{
	return ActionName;
}

bool ADoor::HasOnInteractionCallback()
{
	return true;
}

FDelegateHandle ADoor::AddOnInteractionDelegate(UObject* Object, const FName& FunctionName)
{
	return OnInteraction.AddUFunction(Object, FunctionName);
}

void ADoor::RemoveOnInteractionDelegate(FDelegateHandle DelegateHandle)
{
	OnInteraction.Remove(DelegateHandle);
}

void ADoor::OnLevelDeserialized_Implementation()
{
	const float NewYawAngle = bIsOpen ? MinMaxAnimAngles.Y : MinMaxAnimAngles.X;
	DoorPivot->SetRelativeRotation(FRotator(0.f, NewYawAngle, 0.f));
}

void ADoor::UpdateDoorAnimTimeline(float Alpha) const
{
	const float NewYawAngle = FMath::Lerp(MinMaxAnimAngles.X, MinMaxAnimAngles.Y, FMath::Clamp(Alpha, 0.0f, 1.0f));
	DoorPivot->SetRelativeRotation(FRotator(0.f, NewYawAngle, 0.f));
}

void ADoor::OnDoorAnimFinished()
{
	SetActorTickEnabled(false);
}
