// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "GameFramework/Actor.h"
#include "EquipmentItem.generated.h"

class AXyzBaseCharacter;

UCLASS()
class XYZHOMEWORK_API AEquipmentItem : public AActor
{
	GENERATED_BODY()

public:
	AEquipmentItem();
	virtual void SetOwner(AActor* NewOwner) override;
	EEquipmentItemType GetItemType() const { return EquipmentItemType; };
	FName GetEquippedSocketName() const { return EquippedSocketName; }
	FName GetUnequippedSocketName() const { return UnequippedSocketName; }
	UAnimMontage* GetEquipItemAnimMontage() const { return EquipItemAnimMontage; }
	EReticleType GetReticleType() const { return ReticleType; }
	EReticleType GetAimingReticleType() const { return AimingReticleType; }
	bool IsEquipped() const { return bIsEquipped; }
	void SetIsEquipped(bool const bIsEquipped_In) { bIsEquipped = bIsEquipped_In; }
	bool CanAimWithThisItem() const { return bCanAimWithThisItem; }
	float GetAimingWalkSpeed() const { return AimingWalkSpeed; }
	float GetAimingFOV() const { return AimingFOV; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Parameters | General")
	FName EquippedSocketName = NAME_None;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Parameters | General")
	FName UnequippedSocketName = NAME_None;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Parameters | General")
	EEquipmentItemType EquipmentItemType = EEquipmentItemType::None;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Parameters | General")
	UAnimMontage* EquipItemAnimMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Parameters | Reticle")
	EReticleType ReticleType = EReticleType::None;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Parameters | Reticle")
	EReticleType AimingReticleType = EReticleType::Default;
	UPROPERTY(EditAnywhere, Category = "Item Parameters | Aiming")
	bool bCanAimWithThisItem = true;
	UPROPERTY(EditAnywhere, Category = "Item Parameters | Aiming", meta = (ClampMin = 0.f, UIMin = 0.f))
	float AimingWalkSpeed = 200.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Parameters | Aiming", meta = (ClampMin = 0.f, UIMin = 0.f, ClampMax = 120.f, UIMax = 120.f))
	float AimingFOV = 90.f;

	TWeakObjectPtr<AXyzBaseCharacter> CachedBaseCharacterOwner;
	bool bIsEquipped = false;
};
