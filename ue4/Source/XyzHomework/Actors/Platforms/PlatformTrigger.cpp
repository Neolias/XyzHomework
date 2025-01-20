// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Platforms/PlatformTrigger.h"

#include "BasePlatform.h"
#include "Characters/XyzBaseCharacter.h"
#include "Net/UnrealNetwork.h"

APlatformTrigger::APlatformTrigger()
{
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);
	NetUpdateFrequency = 2.f;
	MinNetUpdateFrequency = 2.f;

	USceneComponent* TriggerRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = TriggerRoot;

	TriggerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TriggerMesh"));
	TriggerMesh->SetupAttachment(TriggerRoot);

	TriggerCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerCollision"));
	TriggerCollision->SetupAttachment(TriggerRoot);
}

void APlatformTrigger::BeginPlay()
{
	Super::BeginPlay();

	TriggerCollision->OnComponentBeginOverlap.AddDynamic(this, &APlatformTrigger::RegisterPawns);
	TriggerCollision->OnComponentEndOverlap.AddDynamic(this, &APlatformTrigger::UnRegisterPawns);
}

void APlatformTrigger::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APlatformTrigger, bIsTriggered);
}

void APlatformTrigger::SetIsTriggered(const bool bIsTriggered_In)
{
	bIsTriggered = bIsTriggered_In;
	OnSetIsTriggered();
}

void APlatformTrigger::OnSetIsTriggered() const
{
	if (OnTriggerStatusChanged.IsBound())
	{
		OnTriggerStatusChanged.Broadcast(bIsTriggered);
	}

	for (ABasePlatform* Platform : ControlledPlatforms)
	{
		if (!IsValid(Platform))
		{
			continue;
		}

		if (bIsTriggered)
		{
			Platform->InvokePlatform();
		}
		else
		{
			Platform->ResetPlatform();
		}
	}
}

void APlatformTrigger::OnRep_SetIsTriggered(bool bIsTriggered_Old)
{
	OnSetIsTriggered();
}

void APlatformTrigger::RegisterPawns(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AXyzBaseCharacter* Character = Cast<AXyzBaseCharacter>(OtherActor);
	if (!IsValid(Character))
	{
		return;
	}

	OverlappedPawns.AddUnique(Character);

	if (!bIsTriggered && OverlappedPawns.Num() > 0)
	{
		if (GetLocalRole() == ROLE_Authority)
		{
			SetIsTriggered(true);
		}
	}
}

void APlatformTrigger::UnRegisterPawns(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AXyzBaseCharacter* Character = Cast<AXyzBaseCharacter>(OtherActor);
	if (!IsValid(Character))
	{
		return;
	}

	OverlappedPawns.RemoveSingleSwap(Character);

	if (bIsTriggered && OverlappedPawns.Num() < 1)
	{
		if (GetLocalRole() == ROLE_Authority)
		{
			SetIsTriggered(false);
		}
	}
}
