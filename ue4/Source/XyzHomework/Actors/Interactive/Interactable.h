// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

UINTERFACE(MinimalAPI)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};


class XYZHOMEWORK_API IInteractable
{
	GENERATED_BODY()

public:
	virtual void Interact(APawn* InteractingPawn) PURE_VIRTUAL(IInteractable::Interact, )
};
