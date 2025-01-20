// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Interactive/InteractiveActor.h"

#include "Characters/XyzBaseCharacter.h"
#include "Components/CapsuleComponent.h"

void AInteractiveActor::BeginPlay()
{
	Super::BeginPlay();
	InteractiveVolume->OnComponentBeginOverlap.AddDynamic(this, &AInteractiveActor::OnInteractiveVolumeBeginOverlap);
	InteractiveVolume->OnComponentEndOverlap.AddDynamic(this, &AInteractiveActor::OnInteractiveVolumeEndOverlap);
}

void AInteractiveActor::OnInteractiveVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(OtherActor);
	if (!IsValid(BaseCharacter) || !OverlapsBaseCharacterCapsuleComponent(BaseCharacter, OtherComp))
	{
		return;
	}
	BaseCharacter->RegisterInteractiveActor(this);
}

void AInteractiveActor::OnInteractiveVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(OtherActor);
	if (!IsValid(BaseCharacter) || !OverlapsBaseCharacterCapsuleComponent(BaseCharacter, OtherComp))
	{
		return;
	}
	BaseCharacter->UnRegisterInteractiveActor(this);
}

bool AInteractiveActor::OverlapsBaseCharacterCapsuleComponent(AXyzBaseCharacter* BaseCharacter, UPrimitiveComponent* Component)
{
	if (Component != BaseCharacter->GetCapsuleComponent())
	{
		return false;
	}
	return true;
}
