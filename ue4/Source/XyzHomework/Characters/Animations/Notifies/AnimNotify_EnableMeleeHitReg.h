// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_EnableMeleeHitReg.generated.h"

/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API UAnimNotify_EnableMeleeHitReg : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee Weapon")
	bool bIsHitRegistrationEnabled = false;
};
