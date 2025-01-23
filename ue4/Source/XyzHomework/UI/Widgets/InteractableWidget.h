// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractableWidget.generated.h"

/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API UInteractableWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetKeyName(FName KeyName);

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* KeyText;	
};
