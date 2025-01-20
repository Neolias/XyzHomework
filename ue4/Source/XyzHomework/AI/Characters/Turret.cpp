// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Characters/Turret.h"

#include "AIController.h"
#include "GenericTeamAgentInterface.h"
#include "Components/AIComponents/TurretAttributesComponent.h"
#include "Components/AIComponents/TurretMuzzleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystem.h"

ATurret::ATurret()
{
	PrimaryActorTick.bCanEverTick = true;

	TurretAttributesComponent = CreateDefaultSubobject<UTurretAttributesComponent>(TEXT("TurretAttributesComponent"));

	USceneComponent* TurretRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TurretRoot"));
	SetRootComponent(TurretRoot);

	BaseMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMeshComponent->SetupAttachment(TurretRoot);

	HeadBaseComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HeadBase"));
	HeadBaseComponent->SetupAttachment(TurretRoot);

	HeadMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	HeadMeshComponent->SetupAttachment(HeadBaseComponent);

	BarrelBaseComponent = CreateDefaultSubobject<USceneComponent>(TEXT("BarrelBase"));
	BarrelBaseComponent->SetupAttachment(HeadBaseComponent);

	BarrelBodyComponent = CreateDefaultSubobject<USceneComponent>(TEXT("BarrelBody"));
	BarrelBodyComponent->SetupAttachment(BarrelBaseComponent);

	BarrelMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrelMesh"));
	BarrelMeshComponent->SetupAttachment(BarrelBodyComponent);

	TurretMuzzleComponent = CreateDefaultSubobject<UTurretMuzzleComponent>(TEXT("TurretMuzzleComponent"));
	TurretMuzzleComponent->SetupAttachment(BarrelBodyComponent);

	SetReplicates(true);
	BaseMeshComponent->SetIsReplicated(true);
	HeadBaseComponent->SetIsReplicated(true);
}

void ATurret::BeginPlay()
{
	Super::BeginPlay();

	OnTakeAnyDamage.AddDynamic(TurretAttributesComponent, &UTurretAttributesComponent::OnDamageTaken);
	TurretAttributesComponent->OnDeathEvent.AddDynamic(this, &ATurret::OnDeath);

	TurretMeshHeight = BaseHeight + HeadHeight;
}

void ATurret::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	const UStaticMesh* BaseMesh = BaseMeshComponent->GetStaticMesh();
	if (IsValid(BaseMesh))
	{
		const FVector BaseMeshSize = BaseMesh->GetBoundingBox().GetSize();
		if (!FMath::IsNearlyZero(BaseMeshSize.X) && !FMath::IsNearlyZero(BaseMeshSize.Y) && !FMath::IsNearlyZero(BaseMeshSize.Z))
		{
			BaseMeshComponent->SetRelativeScale3D(FVector(BaseDiameter / BaseMeshSize.X, BaseDiameter / BaseMeshSize.Y, BaseHeight / BaseMeshSize.Z));
		}
	}

	const UStaticMesh* HeadMesh = HeadMeshComponent->GetStaticMesh();
	if (IsValid(HeadMesh))
	{
		const FVector HeadMeshSize = HeadMesh->GetBoundingBox().GetSize();
		if (!FMath::IsNearlyZero(HeadMeshSize.X) && !FMath::IsNearlyZero(HeadMeshSize.Y) && !FMath::IsNearlyZero(HeadMeshSize.Z))
		{
			HeadMeshComponent->SetRelativeScale3D(FVector(HeadDiameter / HeadMeshSize.X, HeadDiameter / HeadMeshSize.Y, HeadHeight / HeadMeshSize.Z));
		}
	}

	const UStaticMesh* BarrelMesh = BarrelMeshComponent->GetStaticMesh();
	if (IsValid(BarrelMesh))
	{
		const FVector BarrelMeshSize = BarrelMesh->GetBoundingBox().GetSize();
		if (!FMath::IsNearlyZero(BarrelMeshSize.X) && !FMath::IsNearlyZero(BarrelMeshSize.Y) && !FMath::IsNearlyZero(BarrelMeshSize.Z))
		{
			BarrelMeshComponent->SetRelativeScale3D(FVector(BarrelDiameter / BarrelMeshSize.X, BarrelDiameter / BarrelMeshSize.Y, BarrelLength / BarrelMeshSize.Z));
		}
	}

	BaseMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, BaseHeight / 2));
	HeadBaseComponent->SetRelativeLocation(FVector(0.f, 0.f, BaseHeight + HeadHeight / 2));
	BarrelBaseComponent->SetRelativeLocation(FVector(HeadDiameter / 2, 0.f, 0.f));
	BarrelBaseComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	const float OffsetFromHead = -BarrelDiameter;
	BarrelMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, BarrelLength / 2 + OffsetFromHead));
	TurretMuzzleComponent->SetRelativeLocation(FVector(0.f, 0.f, BarrelLength + OffsetFromHead));
	TurretMuzzleComponent->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));

	TurretMeshHeight = BaseHeight + HeadHeight;
}

void ATurret::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AAIController* AIController = Cast<AAIController>(NewController);
	if (IsValid(AIController))
	{
		const FGenericTeamId TeamId = (uint8)Team;
		AIController->SetGenericTeamId(TeamId);
	}
}

void ATurret::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATurret, CurrentTarget);
}

void ATurret::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (CurrentTurretMode)
	{
	default:
	case ETurretMode::Dead:
		break;
	case ETurretMode::Searching:
		SearchEnemy(DeltaTime);
		break;
	case ETurretMode::Tracking:
		TrackEnemy(DeltaTime);
		break;
	}
}

void ATurret::SetTurretMode(const ETurretMode NewMode)
{
	if (NewMode == CurrentTurretMode)
	{
		return;
	}

	if (CurrentTurretMode != ETurretMode::Dead)
	{
		CurrentTurretMode = NewMode;
	}

	switch (CurrentTurretMode)
	{
	default:
	case ETurretMode::Dead:
	case ETurretMode::Searching:
	{
		CurrentTarget = nullptr;
		TurretMuzzleComponent->StopFire();
		Multicast_StopFire();
		break;
	}
	case ETurretMode::Tracking:
	{
		TurretMuzzleComponent->StartFire();
		Multicast_StartFire();
		break;
	}
	}
}

void ATurret::Multicast_StartFire_Implementation()
{
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		TurretMuzzleComponent->StartFire();
	}
}

void ATurret::Multicast_StopFire_Implementation()
{
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		TurretMuzzleComponent->StopFire();
	}
}

void ATurret::SetCurrentTarget(AActor* NewTarget)
{
	CurrentTarget = NewTarget;
	OnCurrentTargetSet();
}

void ATurret::OnCurrentTargetSet()
{
	const ETurretMode NewMode = CurrentTarget.IsValid() ? ETurretMode::Tracking : ETurretMode::Searching;
	SetTurretMode(NewMode);
}

void ATurret::OnRep_CurrentTargetSet()
{
	OnCurrentTargetSet();
}

FVector ATurret::GetPawnViewLocation() const
{
	return BarrelBaseComponent->GetComponentLocation();
}

FRotator ATurret::GetViewRotation() const
{
	return BarrelBaseComponent->GetUpVector().ToOrientationRotator();
}

FGenericTeamId ATurret::GetGenericTeamId() const
{
	return FGenericTeamId((uint8)Team);
}

void ATurret::SearchEnemy(const float DeltaTime) const
{
	FRotator NewHeadRotation = HeadBaseComponent->GetRelativeRotation();
	NewHeadRotation.Yaw = FMath::FInterpTo(NewHeadRotation.Yaw, NewHeadRotation.Yaw + 90.f, DeltaTime, HeadSearchRotationSpeed);
	HeadBaseComponent->SetRelativeRotation(NewHeadRotation);
}

void ATurret::TrackEnemy(const float DeltaTime)
{
	if (!CurrentTarget.IsValid())
	{
		CurrentTurretMode = ETurretMode::Searching;
		return;
	}

	FVector DesiredHeadDirection = CurrentTarget->GetActorLocation() - GetActorLocation() - HeadBaseComponent->GetRelativeLocation();
	DesiredHeadDirection.Z = 0.f;

	const FRotator CurrentHeadRotation = HeadBaseComponent->GetRelativeRotation();
	const FRotator NewHeadRotation = FMath::RInterpTo(CurrentHeadRotation, DesiredHeadDirection.ToOrientationRotator(), DeltaTime, HeadTrackRotationSpeed);
	HeadBaseComponent->SetRelativeRotation(NewHeadRotation);

	const FVector DesiredBarrelDirection = CurrentTarget->GetActorLocation() - BarrelBaseComponent->GetComponentLocation();
	float DesiredBarrelPitch = DesiredBarrelDirection.ToOrientationRotator().Pitch;
	DesiredBarrelPitch = FMath::Clamp(DesiredBarrelPitch, BarrelMinPitchAngle, BarrelMaxPitchAngle);

	const FRotator CurrentBarrelBaseRotation = BarrelBodyComponent->GetRelativeRotation();
	const float NewBarrelBasePitch = FMath::FInterpTo(CurrentBarrelBaseRotation.Pitch, DesiredBarrelPitch, DeltaTime, BarrelPitchRotationSpeed);
	const FRotator NewBarrelBaseRotation = FRotator(NewBarrelBasePitch, 0.f, 0.f);
	BarrelBodyComponent->SetRelativeRotation(NewBarrelBaseRotation);
}

void ATurret::OnDeath()
{
	Multicast_OnDeath();
}

void ATurret::Multicast_OnDeath_Implementation()
{
	SetActorEnableCollision(false);
	BaseMeshComponent->SetVisibility(false, true);
	HeadBaseComponent->SetVisibility(false, true);

	SetTurretMode(ETurretMode::Dead);

	if (IsValid(ExplosionVFX))
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionVFX, GetActorLocation());
	}
}