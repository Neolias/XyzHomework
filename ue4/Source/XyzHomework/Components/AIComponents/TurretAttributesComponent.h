// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TurretAttributesComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathEvent);

class ATurret;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XYZHOMEWORK_API UTurretAttributesComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	FOnDeathEvent OnDeathEvent;

	UTurretAttributesComponent();
	float GetCurrentHealth() const { return CurrentHealth; }
	bool IsAlive() const;
	UFUNCTION()
	void OnDamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Attributes | Debugging", meta = (ClampMin = 0.f, UIMin = 0.f))
	float DefaultPlayerDistanceFromCamera = 175.f;
	UPROPERTY(EditDefaultsOnly, Category = "Attributes | Debugging", meta = (ClampMin = 0.f, UIMin = 0.f))
	float AttributesVisibilityRange = 1000.f;
	UPROPERTY(EditDefaultsOnly, Category = "Attributes | Debugging", meta = (ClampMin = 0.f, UIMin = 0.f))
	float AttributesFontSize = 1.5f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes | Health")
	float MaxHealth = 100.f;
	UPROPERTY(EditAnywhere, Category = "Attributes | Health")
	FVector HealthBarOffset = FVector(0.f, -10.f, 35.f);

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	TWeakObjectPtr<ATurret> CachedTurret;
	float CurrentHealth = 0.f;
	bool bIsDeathTriggered = false;

	void TryTriggerDeath();

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	void DrawDebugAttributes() const;
#endif
};
