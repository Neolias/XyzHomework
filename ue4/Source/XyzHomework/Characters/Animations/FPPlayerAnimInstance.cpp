// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Animations/FPPlayerAnimInstance.h"

#include "Characters/FPPlayerCharacter.h"

void UFPPlayerAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	checkf(TryGetPawnOwner()->IsA<AFPPlayerCharacter>(), TEXT("UFPPlayerAnimInstance::NativeBeginPlay() should be used only with AFPPlayerCharacter class."))
		CachedFPCharacter = StaticCast<AFPPlayerCharacter*>(TryGetPawnOwner());
}

void UFPPlayerAnimInstance::NativeUpdateAnimation(const float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!CachedFPCharacter.IsValid())
	{
		return;
	}
}
