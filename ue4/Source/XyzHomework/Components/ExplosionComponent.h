// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "ExplosionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExplostionEvent);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class XYZHOMEWORK_API UExplosionComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Explosion Component")
	FOnExplostionEvent OnExplosionEvent;
	virtual void BeginPlay() override;
	void Explode(AController* Controller) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 1.f, UIMin = 1.f), Category = "Explosion | Damage")
	float BaseDamage = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 1.f, UIMin = 1.f), Category = "Explosion | Damage")
	float MinimumDamage = 20.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 1.f, UIMin = 1.f), Category = "Explosion | Damage")
	float InnerRadius = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 1.f, UIMin = 1.f), Category = "Explosion | Damage")
	float OuterRadius = 500.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 1.f, UIMin = 1.f), Category = "Explosion | Damage")
	float DamageFalloff = 1.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion | Damage")
	TSubclassOf<UDamageType> DamageType = UDamageType::StaticClass();
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion | FX")
	UParticleSystem* ExplosionVFX;

	APawn* GetOwningPawn() const;
	UFUNCTION()
	void OnDamageTaken(AActor* DamagedActor, float Damage, const class UDamageType* DamageType_In, class AController* InstigatedBy, AActor* DamageCauser);
};
