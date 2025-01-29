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

	DECLARE_MULTICAST_DELEGATE(FOnInteraction)

public:
	virtual void Interact(APawn* InteractingPawn) PURE_VIRTUAL(IInteractable::Interact, );
	virtual FName GetActionName() PURE_VIRTUAL(IInteractable::GetActionName, return FName(NAME_None););
	virtual bool HasOnInteractionCallback() PURE_VIRTUAL(IInteractable::HasOnInteractionCallback, return false; );
	virtual FDelegateHandle AddOnInteractionDelegate(UObject* Object, const FName& FunctionName) PURE_VIRTUAL(IInteractable::AddOnInteractionDelegate, return FDelegateHandle(); );
	virtual void RemoveOnInteractionDelegate(FDelegateHandle DelegateHandle) PURE_VIRTUAL(IInteractable::RemoveOnInteractionDelegate, );

	FOnInteraction OnInteraction;
};
