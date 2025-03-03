// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/XyzAbilitySystemComponent.h"

// 123
bool UXyzAbilitySystemComponent::TryActivateAbilityWithTag(FGameplayTag GameplayTag, bool bAllowRemoteActivation/* = true*/)
{
	return TryActivateAbilitiesByTag(FGameplayTagContainer(GameplayTag), bAllowRemoteActivation);
}

bool UXyzAbilitySystemComponent::TryCancelAbilityWithTag(FGameplayTag GameplayTag)
{
	bool Result = false;
	TArray<FGameplayAbilitySpec*> Abilities;
	GetActivatableGameplayAbilitySpecsByAllMatchingTags(FGameplayTagContainer(GameplayTag), Abilities, false);

	for (FGameplayAbilitySpec* AbilitySpec : Abilities)
	{
		TArray<UGameplayAbility*> AbilityInstances = AbilitySpec->GetAbilityInstances();
		for (UGameplayAbility* Ability : AbilityInstances)
		{
			Ability->K2_CancelAbility();
			Result = true;
		}
	}
	return Result;
}

bool UXyzAbilitySystemComponent::IsAbilityActive(FGameplayTag GameplayTag)
{
	TArray<FGameplayAbilitySpec*> Abilities;
	GetActivatableGameplayAbilitySpecsByAllMatchingTags(FGameplayTagContainer(GameplayTag), Abilities, false);

	for (FGameplayAbilitySpec* AbilitySpec : Abilities)
	{
		TArray<UGameplayAbility*> AbilityInstances = AbilitySpec->GetAbilityInstances();
		for (UGameplayAbility* Ability : AbilityInstances)
		{
			if (Ability->IsActive())
			{
				return true;
			}
		}
	}
	return false;
}
