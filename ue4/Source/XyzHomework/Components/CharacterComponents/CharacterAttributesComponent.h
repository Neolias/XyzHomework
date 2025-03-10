// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Subsystems/SaveSubsystem/SaveSubsystemInterface.h"
#include "CharacterAttributesComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnDeathEventDelegate, bool);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOutOfStaminaEvent, bool, bIsOutOfStamina);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnHealthChangedEvent, float)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnStaminaChangedEvent, float)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnOxygenChangedEvent, float)

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XYZHOMEWORK_API UCharacterAttributesComponent : public UActorComponent, public ISaveSubsystemInterface
{
	GENERATED_BODY()

public:
	FOnDeathEventDelegate OnDeathEvent;
	FOnOutOfStaminaEvent OnOutOfStaminaEvent;
	FOnHealthChangedEvent OnHealthChangedEvent;
	FOnStaminaChangedEvent OnStaminaChangedEvent;
	FOnOxygenChangedEvent OnOxygenChangedEvent;

	explicit UCharacterAttributesComponent();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	float GetMaxStamina() const { return MaxStamina; }
	float GetCurrentStamina() const { return CurrentStamina; }
	float GetHealthPercentage() const { return CurrentHealth / MaxHealth; }
	void AddHealth(float Amount);
	void SetCurrentHealth(const float NewHealth);
	void SetCurrentStamina(const float NewStamina);
	UFUNCTION(BlueprintCallable, Category = "Character Attributes Component")
	bool IsAlive() const;
	bool IsOutOfStamina() const { return bIsOutOfStamina; }
	bool IsOutOfOxygen() const;
	void TakeFallDamage(float FallHeight) const;

	//@ SaveSubsystemInterface
	virtual void OnLevelDeserialized_Implementation() override;
	//~ SaveSubsystemInterface

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Attributes | Debugging", meta = (ClampMin = 0.f, UIMin = 0.f))
	float DefaultPlayerDistanceFromCamera = 175.f;
	UPROPERTY(EditDefaultsOnly, Category = "Attributes | Debugging", meta = (ClampMin = 0.f, UIMin = 0.f))
	float AttributesVisibilityRange = 1000.f;
	UPROPERTY(EditDefaultsOnly, Category = "Attributes | Debugging", meta = (ClampMin = 0.f, UIMin = 0.f))
	float AttributesFontSize = 1.5f;
	UPROPERTY(EditDefaultsOnly, Category = "Attributes | Health", meta = (ClampMin = 1.f, UIMin = 1.f))
	float MaxHealth = 100.f;
	UPROPERTY(EditAnywhere, Category = "Attributes | Health")
	FVector HealthBarOffset = FVector(0.f, -10.f, 35.f);
	UPROPERTY(EditDefaultsOnly, Category = "Attributes | Health")
	UCurveFloat* FallDamageCurve;
	UPROPERTY(EditAnywhere, Category = "Attributes | Stamina")
	FVector StaminaBarOffset = FVector(0.f, -10.f, 25.f);
	UPROPERTY(EditAnywhere, Category = "Attributes | Stamina", meta = (ClampMin = 1.f, UIMin = 1.f))
	float MaxStamina = 100.f;
	UPROPERTY(EditAnywhere, Category = "Attributes | Stamina", meta = (ClampMin = 0.f, UIMin = 0.f))
	float StaminaRestoreVelocity = 20.f;
	UPROPERTY(EditAnywhere, Category = "Attributes | Stamina", meta = (ClampMin = 0.f, UIMin = 0.f))
	float SprintStaminaConsumptionVelocity = 20.f;
	UPROPERTY(EditAnywhere, Category = "Attributes | Oxygen")
	FVector OxygenBarOffset = FVector(0.f, -10.f, 15.f);
	UPROPERTY(EditAnywhere, Category = "Attributes | Oxygen", meta = (ClampMin = 1.f, UIMin = 1.f))
	float MaxOxygen = 50.f;
	UPROPERTY(EditAnywhere, Category = "Attributes | Oxygen", meta = (ClampMin = 0.f, UIMin = 0.f))
	float OxygenRestoreVelocity = 15.f;
	UPROPERTY(EditAnywhere, Category = "Attributes | Oxygen", meta = (ClampMin = 0.f, UIMin = 0.f))
	float SwimOxygenConsumptionVelocity = 2.f;
	UPROPERTY(EditAnywhere, Category = "Attributes | Oxygen", meta = (ClampMin = 0.f, UIMin = 0.f))
	float OutOfOxygenDamage = 5.f;
	UPROPERTY(EditAnywhere, Category = "Attributes | Oxygen", meta = (ClampMin = 0.f, UIMin = 0.f))
	float OutOfOxygenDamageRate = 2.f;

private:
	TWeakObjectPtr<class AXyzBaseCharacter> CachedBaseCharacter;
	UPROPERTY()
	class UXyzBaseCharMovementComponent* BaseCharacterMovementComponent;
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, SaveGame)
	float CurrentHealth = 0.f;
	UFUNCTION()
	void OnRep_CurrentHealth();
	UPROPERTY(Replicated)
	float CurrentStamina = 0.f;
	bool bIsOutOfStamina = false;
	UPROPERTY(Replicated)
	float CurrentOxygen = 0.f;
	bool bIsOutOfOxygen = false;
	bool bIsDeathTriggered = false;
	FTimerHandle OutOfOxygenPainTimer;

	UFUNCTION()
	void OnDamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
	void OnHealthChanged();
	void TryTriggerDeath(const UDamageType* DamageType, const bool bShouldPlayAnimation = false);
	void TryTriggerDeath(const bool bShouldPlayAnimation = false);
	void OnDeath(bool bShouldPlayAnimation = false);
	void UpdateStaminaValue(float DeltaTime);
	void OnStaminaChanged();
	void TryChangeOutOfStaminaState();
	void UpdateOxygenValue(float DeltaTime);
	void OnOxygenChanged();
	void TryToggleOutOfOxygenPain();
	void TakeOutOfOxygenDamage() const;

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	void DrawDebugAttributes() const;
#endif
};
