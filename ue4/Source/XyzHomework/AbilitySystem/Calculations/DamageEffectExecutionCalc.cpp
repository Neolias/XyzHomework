// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Calculations/DamageEffectExecutionCalc.h"

#include "AbilitySystemComponent.h"
#include "XyzHomeworkTypes.h"
#include "AbilitySystem/AttributeSets/XyzCharacterAttributeSet.h"

struct XyzDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Health);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Defence);

	XyzDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UXyzCharacterAttributeSet, Health, Target, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UXyzCharacterAttributeSet, Defence, Target, true);
	}
};

static const XyzDamageStatics& DamageStatics()
{
	static XyzDamageStatics DamageStatics;
	return DamageStatics;
}

UDamageEffectExecutionCalc::UDamageEffectExecutionCalc()
{
	RelevantAttributesToCapture.Add(DamageStatics().HealthDef);
	RelevantAttributesToCapture.Add(DamageStatics().DefenceDef);
}

void UDamageEffectExecutionCalc::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float Defence = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DefenceDef, EvaluationParameters, Defence);
	float Damage = -Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(AbilitiesAttributeHealth), false, 0.f);
	float TotalDamage = FMath::Max<float>(Damage - Defence, 0.f);
	if (TotalDamage > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics().HealthProperty, EGameplayModOp::Additive, -TotalDamage));
	}
}
