// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlatform.h"

#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"

ABasePlatform::ABasePlatform()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* PlatformRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = PlatformRoot;

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
	PlatformMesh->SetupAttachment(PlatformRoot);

	PlatformCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerCollision"));
	PlatformCollision->SetupAttachment(PlatformRoot);

	SetReplicates(true);
	NetUpdateFrequency = 2.f;
	MinNetUpdateFrequency = 2.f;
}

void ABasePlatform::BeginPlay()
{
	Super::BeginPlay();
	StartLocation = RootComponent->GetRelativeLocation();

	if (IsValid(TimelineCurve))
	{
		FOnTimelineFloatStatic PlatformMovementTimelineUpdate;
		PlatformMovementTimelineUpdate.BindUObject(this, &ABasePlatform::PlatformTimelineUpdate);
		PlatformTimeline.AddInterpFloat(TimelineCurve, PlatformMovementTimelineUpdate);
	}
}

void ABasePlatform::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	PlatformTimeline.TickTimeline(DeltaSeconds);
}

void ABasePlatform::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABasePlatform, bIsActivated);
}

void ABasePlatform::SetIsActivated(const bool bIsActivated_In)
{
	bIsActivated = bIsActivated_In;
	OnSetIsActivated();
}

void ABasePlatform::OnSetIsActivated() const
{
	if (OnPlatformStatusChanged.IsBound())
	{
		OnPlatformStatusChanged.Broadcast(bIsActivated);
	}
}

void ABasePlatform::OnRep_SetIsActivated(bool bIsActivated_Old)
{
	OnSetIsActivated();
}

void ABasePlatform::InvokePlatform()
{
	SetIsActivated(true);
	PlatformTimelinePlay();
}

void ABasePlatform::ResetPlatform()
{
	SetIsActivated(false);
	PlatformTimeline.Stop();
	PlatformTimelineReverse();
}

void ABasePlatform::PlatformTimelineUpdate(const float Alpha)
{
	const FVector PlatformTargetLocation = FMath::Lerp(StartLocation, StartLocation + EndLocation, Alpha);
	RootComponent->SetRelativeLocation(PlatformTargetLocation);
}
