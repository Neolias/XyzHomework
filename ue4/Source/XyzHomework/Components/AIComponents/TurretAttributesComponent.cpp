// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AIComponents/TurretAttributesComponent.h"

#include "DrawDebugHelpers.h"
#include "XyzHomeworkTypes.h"
#include "AI/Characters/Turret.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/DebugSubsystem.h"

UTurretAttributesComponent::UTurretAttributesComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTurretAttributesComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(GetOwner()->IsA<ATurret>(), TEXT("UTurretAttributesComponent::BeginPlay() UTurretAttributesComponent should be used only with ATurret"))
		CachedTurret = StaticCast<ATurret*>(GetOwner());

	CurrentHealth = MaxHealth;

}

void UTurretAttributesComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	DrawDebugAttributes();
#endif
}

bool UTurretAttributesComponent::IsAlive() const
{
	return !bIsDeathTriggered && CurrentHealth > 0.f;
}

void UTurretAttributesComponent::OnDamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (IsAlive())
	{
		CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);
		TryTriggerDeath();
	}
}

void UTurretAttributesComponent::TryTriggerDeath()
{
	if (!IsAlive())
	{
		bIsDeathTriggered = true;

		if (OnDeathEvent.IsBound())
		{
			OnDeathEvent.Broadcast();
		}
	}
}

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
void UTurretAttributesComponent::DrawDebugAttributes() const
{
	const UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	if (!DebugSubsystem->IsCategoryEnabled(DebugCategoryAIAttributes))
	{
		return;
	}

	const APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	const FVector CameraLocation = CameraManager->GetCameraLocation();
	const FVector TurretLocation = CachedTurret->GetActorLocation();
	const float DistanceFromCamera = FVector::Dist(TurretLocation, CameraLocation);
	if (DistanceFromCamera > AttributesVisibilityRange)
	{
		return;
	}
	const float AttributeFontScale = FMath::Clamp(DefaultPlayerDistanceFromCamera / DistanceFromCamera, 0.5f, 1.f);
	const float ScaledAttributeFontSize = AttributesFontSize * AttributeFontScale;
	FRotator CameraRotation = CameraManager->GetCameraRotation();
	CameraRotation.Pitch = 0.f;
	const FVector HealthBarLocation = TurretLocation + CachedTurret->GetTurretMeshHeight() * FVector::UpVector + CameraRotation.RotateVector(HealthBarOffset);
	DrawDebugString(GetWorld(), HealthBarLocation, FString::Printf(TEXT("%.2f"), CurrentHealth), nullptr, FColor::Red, 0.f, true, ScaledAttributeFontSize);
}
#endif

