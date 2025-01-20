// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Controllers/XyzPlayerController.h"

#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterAttributesComponent.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Widgets/PlayerHUDWidget.h"
#include "UI/Widgets/ReticleWidget.h"
#include "UI/Widgets/WeaponAmmoWidget.h"
#include "UI/Widgets/CharacterAttributesWidget.h"

void AXyzPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	//checkf(InPawn->IsA<AXyzBaseCharacter>(), TEXT("AXyzPlayerController::SetPawn() should be used only with AXyzBaseCharacter"))
	CachedBaseCharacter = StaticCast<AXyzBaseCharacter*>(InPawn);
	if (CachedBaseCharacter.IsValid() && CachedBaseCharacter->IsLocallyControlled())
	{
		CreateAndInitializeHUDWidgets();
	}
}

bool AXyzPlayerController::IgnoresFPCameraPitch() const
{
	if (CachedBaseCharacter.IsValid())
	{
		return !CachedBaseCharacter->IsFirstPerson() || bIgnoresFPCameraPitch;
	}
	return true;
}

void AXyzPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAxis("MoveForward", this, &AXyzPlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AXyzPlayerController::MoveRight);
	InputComponent->BindAxis("Turn", this, &AXyzPlayerController::Turn);
	InputComponent->BindAxis("LookUp", this, &AXyzPlayerController::LookUp);
	InputComponent->BindAction("InteractWithLadder", EInputEvent::IE_Pressed, this, &AXyzPlayerController::InteractWithLadder);
	InputComponent->BindAction("InteractWithZipline", EInputEvent::IE_Pressed, this, &AXyzPlayerController::InteractWithZipline);
	InputComponent->BindAction("Mantle", EInputEvent::IE_Pressed, this, &AXyzPlayerController::Mantle);
	InputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &AXyzPlayerController::Jump);
	InputComponent->BindAction("Crouch", EInputEvent::IE_Pressed, this, &AXyzPlayerController::ChangeCrouchState);
	InputComponent->BindAction("Sprint", EInputEvent::IE_Pressed, this, &AXyzPlayerController::StartSprint);
	InputComponent->BindAction("Sprint", EInputEvent::IE_Released, this, &AXyzPlayerController::StopSprint);
	InputComponent->BindAction("Prone", EInputEvent::IE_Pressed, this, &AXyzPlayerController::ChangeProneState);
	InputComponent->BindAxis("SwimForward", this, &AXyzPlayerController::SwimForward);
	InputComponent->BindAxis("SwimRight", this, &AXyzPlayerController::SwimRight);
	InputComponent->BindAction("Dive", EInputEvent::IE_Pressed, this, &AXyzPlayerController::Dive);
	InputComponent->BindAxis("SwimUp", this, &AXyzPlayerController::SwimUp);
	InputComponent->BindAxis("ClimbLadderUp", this, &AXyzPlayerController::ClimbLadderUp);
	InputComponent->BindAction("JumpOffRunnableWall", EInputEvent::IE_Pressed, this, &AXyzPlayerController::JumpOffRunnableWall);
	InputComponent->BindAction("Slide", EInputEvent::IE_Pressed, this, &AXyzPlayerController::Slide);
	InputComponent->BindAction("ReloadLevel", EInputEvent::IE_Pressed, this, &AXyzPlayerController::ReloadLevel);
	InputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &AXyzPlayerController::StartWeaponFire);
	InputComponent->BindAction("Fire", EInputEvent::IE_Released, this, &AXyzPlayerController::StopFire);
	InputComponent->BindAction("Aim", EInputEvent::IE_Pressed, this, &AXyzPlayerController::StartAim);
	InputComponent->BindAction("Aim", EInputEvent::IE_Released, this, &AXyzPlayerController::EndAim);
	InputComponent->BindAction("ReloadWeapon", EInputEvent::IE_Pressed, this, &AXyzPlayerController::ReloadWeapon);
	InputComponent->BindAction("DrawNextItem", EInputEvent::IE_Pressed, this, &AXyzPlayerController::DrawNextEquipmentItem);
	InputComponent->BindAction("DrawPreviousItem", EInputEvent::IE_Pressed, this, &AXyzPlayerController::DrawPreviousEquipmentItem);
	InputComponent->BindAction("TogglePrimaryItem", EInputEvent::IE_Pressed, this, &AXyzPlayerController::TogglePrimaryItem);
	InputComponent->BindAction("ThrowItem", EInputEvent::IE_Pressed, this, &AXyzPlayerController::ThrowItem);
	InputComponent->BindAction("ActivateNextWeaponMode", EInputEvent::IE_Pressed, this, &AXyzPlayerController::ActivateNextWeaponMode);
	InputComponent->BindAction("UsePrimaryMeleeAttack", EInputEvent::IE_Pressed, this, &AXyzPlayerController::UsePrimaryMeleeAttack);
	InputComponent->BindAction("UseSecondaryMeleeAttack", EInputEvent::IE_Pressed, this, &AXyzPlayerController::UseSecondaryMeleeAttack);

	InputComponent->BindAxis("TurnAtRate", this, &AXyzPlayerController::TurnAtRate);
	InputComponent->BindAxis("LookUpAtRate", this, &AXyzPlayerController::LookUpAtRate);
}

void AXyzPlayerController::CreateAndInitializeHUDWidgets()
{
	if (!IsValid(PlayerHUDWidget) && IsValid(PlayerHUDWidgetClass))
	{
		PlayerHUDWidget = CreateWidget<UPlayerHUDWidget>(GetWorld(), PlayerHUDWidgetClass);
		if (IsValid(PlayerHUDWidget))
		{
			PlayerHUDWidget->AddToViewport();
		}
	}

	if (IsValid(PlayerHUDWidget) && CachedBaseCharacter.IsValid())
	{
		UCharacterEquipmentComponent* CharacterEquipmentComponent = CachedBaseCharacter->GetCharacterEquipmentComponent();

		UReticleWidget* ReticleWidget = PlayerHUDWidget->GetReticleWidget();
		if (IsValid(ReticleWidget))
		{
			CachedBaseCharacter->OnAimingStateChanged.AddUFunction(ReticleWidget, FName("OnAimingStateChanged"));
			if (IsValid(CharacterEquipmentComponent))
			{
				CharacterEquipmentComponent->OnEquipmentItemChangedEvent.AddUFunction(ReticleWidget, FName("OnEquippedItemChanged"));
			}
		}

		UWeaponAmmoWidget* WeaponAmmoWidget = PlayerHUDWidget->GetWeaponAmmoWidget();
		if (IsValid(WeaponAmmoWidget))
		{
			if (IsValid(CharacterEquipmentComponent))
			{
				CharacterEquipmentComponent->OnCurrentWeaponAmmoChangedEvent.AddUFunction(WeaponAmmoWidget, FName("OnWeaponAmmoChanged"));
				CharacterEquipmentComponent->OnCurrentThrowableAmmoChangedEvent.AddUFunction(WeaponAmmoWidget, FName("OnThrowableAmmoChanged"));
			}
		}

		UCharacterAttributesWidget* CharacterAttributesWidget = PlayerHUDWidget->GetCharacterAttributesWidget();
		if (IsValid(CharacterAttributesWidget))
		{
			UCharacterAttributesComponent* CharacterAttributesComponent = CachedBaseCharacter->GetCharacterAttributesComponent();
			if (IsValid(CharacterAttributesComponent))
			{
				CharacterAttributesComponent->OnHealthChanged.AddUFunction(CharacterAttributesWidget, FName("OnHealthChanged"));
				CharacterAttributesComponent->OnStaminaChanged.AddUFunction(CharacterAttributesWidget, FName("OnStaminaChanged"));
				CharacterAttributesComponent->OnOxygenChanged.AddUFunction(CharacterAttributesWidget, FName("OnOxygenChanged"));
			}
		}

		UCharacterAttributesWidget* CharacterAttributesCenterWidget = PlayerHUDWidget->GetCharacterAttributesCenterWidget();
		if (IsValid(CharacterAttributesCenterWidget))
		{
			UCharacterAttributesComponent* CharacterAttributesComponent = CachedBaseCharacter->GetCharacterAttributesComponent();
			if (IsValid(CharacterAttributesComponent))
			{
				CharacterAttributesComponent->OnHealthChanged.AddUFunction(CharacterAttributesCenterWidget, FName("OnHealthChanged"));
				CharacterAttributesComponent->OnStaminaChanged.AddUFunction(CharacterAttributesCenterWidget, FName("OnStaminaChanged"));
				CharacterAttributesComponent->OnOxygenChanged.AddUFunction(CharacterAttributesCenterWidget, FName("OnOxygenChanged"));
			}
		}
	}
}

void AXyzPlayerController::MoveForward(const float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->MoveForward(Value);
	}
}

void AXyzPlayerController::MoveRight(const float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->MoveRight(Value);
	}
}

void AXyzPlayerController::Turn(const float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Turn(Value);
	}
}

void AXyzPlayerController::LookUp(const float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->LookUp(Value);
	}
}

void AXyzPlayerController::ChangeCrouchState()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->ChangeCrouchState();
	}
}

void AXyzPlayerController::InteractWithLadder()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->InteractWithLadder();
	}
}

void AXyzPlayerController::InteractWithZipline()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->InteractWithZipline();
	}
}

void AXyzPlayerController::Mantle()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Mantle();
	}
}

void AXyzPlayerController::Jump()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Jump();
	}
}

void AXyzPlayerController::StartSprint()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StartSprint();
	}
}

void AXyzPlayerController::StopSprint()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StopSprint();
	}
}

void AXyzPlayerController::ChangeProneState()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->ChangeProneState();
	}
}

void AXyzPlayerController::SwimForward(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SwimForward(Value);
	}
}

void AXyzPlayerController::SwimRight(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SwimRight(Value);
	}
}

void AXyzPlayerController::SwimUp(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SwimUp(Value);
	}
}

void AXyzPlayerController::Dive()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Dive();
	}
}

void AXyzPlayerController::ClimbLadderUp(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->ClimbLadderUp(Value);
	}
}

void AXyzPlayerController::JumpOffRunnableWall()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->JumpOffRunnableWall();
	}
}

void AXyzPlayerController::Slide()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StartSlide();
	}
}

void AXyzPlayerController::ReloadLevel()
{
	const UWorld* World = GetWorld();
	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(World);
	UGameplayStatics::OpenLevel(World, FName(*CurrentLevelName));
}

void AXyzPlayerController::StartWeaponFire()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StartWeaponFire();
	}
}

void AXyzPlayerController::StopFire()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StopWeaponFire();
	}
}

void AXyzPlayerController::StartAim()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SetWantsToAim();
	}
}

void AXyzPlayerController::EndAim()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->ResetWantsToAim();
	}
}

void AXyzPlayerController::ReloadWeapon()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->ReloadWeapon();
	}
}

void AXyzPlayerController::DrawNextEquipmentItem()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->DrawNextEquipmentItem();
	}
}

void AXyzPlayerController::DrawPreviousEquipmentItem()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->DrawPreviousEquipmentItem();
	}
}

void AXyzPlayerController::TogglePrimaryItem()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->TogglePrimaryItem();
	}
}

void AXyzPlayerController::ThrowItem()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->ThrowItem();
	}
}

void AXyzPlayerController::ActivateNextWeaponMode()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->ActivateNextWeaponMode();
	}
}

void AXyzPlayerController::UsePrimaryMeleeAttack()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->UsePrimaryMeleeAttack();
	}
}

void AXyzPlayerController::UseSecondaryMeleeAttack()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->UseSecondaryMeleeAttack();
	}
}

void AXyzPlayerController::TurnAtRate(const float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->TurnAtRate(Value);
	}
}

void AXyzPlayerController::LookUpAtRate(const float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->LookUpAtRate(Value);
	}
}
