// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "XyzAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API UXyzAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	bool TryActivateAbilityWithTag(FGameplayTag GameplayTag, bool bAllowRemoteActivation = true);
	bool TryCancelAbilityWithTag(FGameplayTag GameplayTag);
	bool IsAbilityActive(FGameplayTag GameplayTag);
};
