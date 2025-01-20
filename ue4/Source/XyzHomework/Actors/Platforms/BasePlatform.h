// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "BasePlatform.generated.h"


UENUM(BlueprintType)
enum class EPlatformBehavior : uint8
{
	OnDemand = 0,
	Loop,
	Default,
	Max UMETA(Hidden)
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlatformStatusChangedEvent, bool, bIsActivated);

/**
 *
 */
UCLASS()
class XYZHOMEWORK_API ABasePlatform : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnPlatformStatusChangedEvent OnPlatformStatusChanged;

	ABasePlatform();
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetIsActivated(bool bIsActivated_In);
	void InvokePlatform();
	void ResetPlatform();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* PlatformMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UBoxComponent* PlatformCollision;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient)
	FVector StartLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector EndLocation;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* TimelineCurve;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.f, UIMin = 0.f))
	float MovementCooldown = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EPlatformBehavior PlatformBehavior = EPlatformBehavior::OnDemand;

	virtual void BeginPlay() override;
	UFUNCTION(BlueprintCallable)
	void PlatformTimelinePlay() { PlatformTimeline.Play(); }
	UFUNCTION(BlueprintCallable)
	void PlatformTimelineReverse() { PlatformTimeline.Reverse(); }

private:
	UPROPERTY(ReplicatedUsing = OnRep_SetIsActivated)
	bool bIsActivated = false;
	FTimeline PlatformTimeline;
	FTimerHandle PlatformIdleTimer;

	void OnSetIsActivated() const;
	UFUNCTION()
	void OnRep_SetIsActivated(bool bIsActivated_Old);
	void PlatformTimelineUpdate(float Alpha);
};
