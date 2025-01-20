// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CharacterComponents/CharacterAttributesComponent.h"

#include "DrawDebugHelpers.h"
#include "XyzHomeworkTypes.h"
#include "DamageTypes/Volumes/PainVolumeDamageType.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/MovementComponents/XyzBaseCharMovementComponent.h"
#include "DamageTypes/Weapons/BulletDamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/DebugSubsystem.h"

UCharacterAttributesComponent::UCharacterAttributesComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UCharacterAttributesComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(GetOwner()->IsA<AXyzBaseCharacter>(), TEXT("UCharacterAttributesComponent::BeginPlay() CharacterAttributesComponent should be used only with AXyzBaseCharacter"))
		CachedBaseCharacter = StaticCast<AXyzBaseCharacter*>(GetOwner());

	BaseCharacterMovementComponent = CachedBaseCharacter->GetBaseCharacterMovementComponent();

	CurrentHealth = MaxHealth;
	CurrentStamina = MaxStamina;
	CurrentOxygen = MaxOxygen;

	CachedBaseCharacter->OnTakeAnyDamage.AddDynamic(this, &UCharacterAttributesComponent::OnDamageTaken);
}

void UCharacterAttributesComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCharacterAttributesComponent, CurrentHealth);
	DOREPLIFETIME(UCharacterAttributesComponent, CurrentStamina);
	DOREPLIFETIME(UCharacterAttributesComponent, CurrentOxygen);
}

void UCharacterAttributesComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateStaminaValue(DeltaTime);
	TryChangeOutOfStaminaState();
	UpdateOxygenValue(DeltaTime);
	TryToggleOutOfOxygenPain();

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	DrawDebugAttributes();
#endif
}

void UCharacterAttributesComponent::OnRep_CurrentHealth()
{
	if (OnHealthChanged.IsBound())
	{
		OnHealthChanged.Broadcast(CurrentHealth / MaxHealth);
	}
	TryTriggerDeath(true);
}

void UCharacterAttributesComponent::OnDamageTaken(AActor* DamagedActor, const float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (IsAlive())
	{
		CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);
		if (OnHealthChanged.IsBound())
		{
			OnHealthChanged.Broadcast(CurrentHealth / MaxHealth);
		}
		TryTriggerDeath(DamageType);
	}
}

void UCharacterAttributesComponent::TryTriggerDeath(const UDamageType* DamageType, const bool bShouldPlayAnimation/* = false*/)
{
	if (!IsAlive())
	{
		bIsDeathTriggered = true;

		if (OnDeathDelegate.IsBound())
		{
			if (bShouldPlayAnimation || DamageType->IsA<UPainVolumeDamageType>() || DamageType->IsA<UBulletDamageType>())
			{
				OnDeathDelegate.Broadcast(true);
			}
			else
			{
				OnDeathDelegate.Broadcast(false);
			}
		}
	}
}

void UCharacterAttributesComponent::TryTriggerDeath(const bool bShouldPlayAnimation)
{
	if (!IsAlive())
	{
		bIsDeathTriggered = true;

		if (OnDeathDelegate.IsBound())
		{
			OnDeathDelegate.Broadcast(bShouldPlayAnimation);
		}
	}
}

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
void UCharacterAttributesComponent::DrawDebugAttributes() const
{
	const UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	if (CachedBaseCharacter->IsFirstPerson() || !DebugSubsystem->IsCategoryEnabled(DebugCategoryCharacterAttributes))
	{
		return;
	}

	const APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	const FVector CameraLocation = CameraManager->GetCameraLocation();
	const FVector CharacterLocation = CachedBaseCharacter->GetActorLocation();
	const float DistanceFromCamera = FVector::Dist(CharacterLocation, CameraLocation);
	if (DistanceFromCamera > AttributesVisibilityRange)
	{
		return;
	}
	const float AttributeFontScale = FMath::Clamp(DefaultPlayerDistanceFromCamera / DistanceFromCamera, 0.5f, 1.f);
	const float ScaledAttributeFontSize = AttributesFontSize * AttributeFontScale;
	FRotator CameraRotation = CameraManager->GetCameraRotation();
	CameraRotation.Pitch = 0.f;
	const FVector HealthBarLocation = CharacterLocation + CachedBaseCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * FVector::UpVector + CameraRotation.RotateVector(HealthBarOffset);
	const FVector StaminaBarLocation = CharacterLocation + CachedBaseCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * FVector::UpVector + CameraRotation.RotateVector(StaminaBarOffset);
	const FVector OxygenBarLocation = CharacterLocation + CachedBaseCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * FVector::UpVector + CameraRotation.RotateVector(OxygenBarOffset);
	DrawDebugString(GetWorld(), HealthBarLocation, FString::Printf(TEXT("%.2f"), CurrentHealth), nullptr, FColor::Red, 0.f, true, ScaledAttributeFontSize);
	DrawDebugString(GetWorld(), StaminaBarLocation, FString::Printf(TEXT("%.2f"), CurrentStamina), nullptr, FColor::Yellow, 0.f, true, ScaledAttributeFontSize);
	DrawDebugString(GetWorld(), OxygenBarLocation, FString::Printf(TEXT("%.2f"), CurrentOxygen), nullptr, FColor::Blue, 0.f, true, ScaledAttributeFontSize);
}
#endif

bool UCharacterAttributesComponent::IsAlive() const
{
	if (!bIsDeathTriggered && CurrentHealth > 0.f)
	{
		return true;
	}
	return false;
}

bool UCharacterAttributesComponent::IsOutOfOxygen() const
{
	if (CurrentOxygen == 0.f)
	{
		return true;
	}
	return false;
}

void UCharacterAttributesComponent::TakeFallDamage(const float FallHeight) const
{
	if (IsValid(FallDamageCurve))
	{
		const float Damage = FallDamageCurve->GetFloatValue(FallHeight);
		CachedBaseCharacter->TakeDamage(Damage, FDamageEvent(), CachedBaseCharacter->GetController(), CachedBaseCharacter.Get());
	}
}

void UCharacterAttributesComponent::UpdateStaminaValue(const float DeltaTime)
{
	const float Delta = BaseCharacterMovementComponent->IsSprinting() ? -SprintStaminaConsumptionVelocity : StaminaRestoreVelocity;
	CurrentStamina += Delta * DeltaTime;
	CurrentStamina = FMath::Clamp(CurrentStamina, 0.f, MaxStamina);
	if (OnStaminaChanged.IsBound())
	{
		OnStaminaChanged.Broadcast(CurrentStamina / MaxStamina);
	}
}

void UCharacterAttributesComponent::TryChangeOutOfStaminaState()
{
	if (!OutOfStaminaEventSignature.IsBound())
	{
		return;
	}

	if (bIsOutOfStamina)
	{
		if (FMath::IsNearlyEqual(CurrentStamina, MaxStamina))
		{
			bIsOutOfStamina = false;
			OutOfStaminaEventSignature.Broadcast(false);
		}
	}
	else if (FMath::IsNearlyZero(CurrentStamina))
	{
		bIsOutOfStamina = true;
		OutOfStaminaEventSignature.Broadcast(true);
	}
}

void UCharacterAttributesComponent::UpdateOxygenValue(const float DeltaTime)
{
	const float Delta = BaseCharacterMovementComponent->IsSwimming() && BaseCharacterMovementComponent->IsSwimmingUnderWater() ? -SwimOxygenConsumptionVelocity : OxygenRestoreVelocity;
	CurrentOxygen += Delta * DeltaTime;
	CurrentOxygen = FMath::Clamp(CurrentOxygen, 0.f, MaxOxygen);
	if (OnOxygenChanged.IsBound())
	{
		OnOxygenChanged.Broadcast(CurrentOxygen / MaxOxygen);
	}
}

void UCharacterAttributesComponent::TryToggleOutOfOxygenPain()
{
	if (IsAlive() && BaseCharacterMovementComponent->IsSwimmingUnderWater() && IsOutOfOxygen())
	{
		if (!CachedBaseCharacter->GetWorldTimerManager().IsTimerActive(OutOfOxygenPainTimer))
		{
			CachedBaseCharacter->GetWorldTimerManager().SetTimer(OutOfOxygenPainTimer, this, &UCharacterAttributesComponent::TakeOutOfOxygenDamage, OutOfOxygenDamageRate, true, 0.f);
		}
	}
	else
	{
		CachedBaseCharacter->GetWorldTimerManager().ClearTimer(OutOfOxygenPainTimer);
	}
}

void UCharacterAttributesComponent::TakeOutOfOxygenDamage() const
{
	CachedBaseCharacter->TakeDamage(OutOfOxygenDamage, FDamageEvent(), CachedBaseCharacter->GetController(), CachedBaseCharacter.Get());
}
