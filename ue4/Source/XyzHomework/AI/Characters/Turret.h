// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "XyzGenericEnums.h"
#include "GameFramework/Pawn.h"
#include "Perception/AIPerceptionTypes.h"
#include "Turret.generated.h"

UCLASS()
class XYZHOMEWORK_API ATurret : public APawn, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ATurret();
	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }
	void SetCurrentTarget(AActor* NewTarget);
	void SetTurretMode(ETurretMode NewMode);
	float GetTurretMeshHeight() const { return TurretMeshHeight; }
	FAISenseAffiliationFilter GetAISenseAffiliationFilter() const { return AISenseAffiliationFilter; }

	virtual FVector GetPawnViewLocation() const override;
	virtual FRotator GetViewRotation() const override;

	// IGenericTeamAgentInterface
	virtual FGenericTeamId GetGenericTeamId() const override;
	// ~IGenericTeamAgentInterface

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret Parameters | Components")
	class UTurretAttributesComponent* TurretAttributesComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret Parameters | Components")
	UStaticMeshComponent* BaseMeshComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret Parameters | Components")
	USceneComponent* HeadBaseComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret Parameters | Components")
	UStaticMeshComponent* HeadMeshComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret Parameters | Components")
	USceneComponent* BarrelBaseComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret Parameters | Components")
	USceneComponent* BarrelBodyComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret Parameters | Components")
	UStaticMeshComponent* BarrelMeshComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret Parameters | Components")
	class UTurretMuzzleComponent* TurretMuzzleComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret Parameters | Perception")
	ETeam Team = ETeam::Enemy;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret Parameters | Perception")
	FAISenseAffiliationFilter AISenseAffiliationFilter;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret Parameters | Geometry", meta = (ClampMin = 1.f, UIMin = 1.f))
	float BaseDiameter = 25.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret Parameters | Geometry", meta = (ClampMin = 1.f, UIMin = 1.f))
	float BaseHeight = 150.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret Parameters | Geometry", meta = (ClampMin = 1.f, UIMin = 1.f))
	float HeadDiameter = 40.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret Parameters | Geometry", meta = (ClampMin = 1.f, UIMin = 1.f))
	float HeadHeight = 20.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret Parameters | Geometry", meta = (ClampMin = 1.f, UIMin = 1.f))
	float BarrelDiameter = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret Parameters | Geometry", meta = (ClampMin = 1.f, UIMin = 1.f))
	float BarrelLength = 40.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret Parameters | Shooting")
	float HeadSearchRotationSpeed = .5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret Parameters | Shooting")
	float HeadTrackRotationSpeed = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret Parameters | Shooting")
	float BarrelPitchRotationSpeed = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret Parameters | Shooting")
	float BarrelMaxPitchAngle = 30.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret Parameters | Shooting")
	float BarrelMinPitchAngle = -30.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Turret Parameters | Death")
	UParticleSystem* ExplosionVFX;

	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	ETurretMode CurrentTurretMode = ETurretMode::Searching;
	UPROPERTY(ReplicatedUsing = OnRep_CurrentTargetSet)
	TWeakObjectPtr<AActor> CurrentTarget;
	FVector TargetLocation = FVector::ZeroVector;
	float TurretMeshHeight = 0.f;

	void SearchEnemy(float DeltaTime) const;
	void TrackEnemy(float DeltaTime);
	UFUNCTION()
	void OnDeath();
	void OnCurrentTargetSet();
	UFUNCTION()
	void OnRep_CurrentTargetSet();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartFire();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopFire();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnDeath();
};
