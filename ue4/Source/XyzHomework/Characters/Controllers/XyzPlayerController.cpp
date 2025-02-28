// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Controllers/XyzPlayerController.h"

#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterAttributesComponent.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "GameFramework/PlayerInput.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/SaveSubsystem/SaveSubsystem.h"
#include "UI/Widgets/PlayerHUD/PlayerHUDWidget.h"
#include "UI/Widgets/PlayerHUD/ReticleWidget.h"
#include "UI/Widgets/PlayerHUD/WeaponAmmoWidget.h"
#include "UI/Widgets/PlayerHUD/CharacterAttributesWidget.h"

void AXyzPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	//checkf(InPawn->IsA<AXyzBaseCharacter>(), TEXT("AXyzPlayerController::SetPawn() should be used only with AXyzBaseCharacter"))
	CachedBaseCharacter = StaticCast<AXyzBaseCharacter*>(InPawn);
	if (CachedBaseCharacter.IsValid() && CachedBaseCharacter->IsLocallyControlled())
	{
		CachedBaseCharacter->OnInteractableFound.AddUObject(this, &AXyzPlayerController::OnInteractableObjectFound);
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
	FInputActionBinding& ToggleMenuBinding = InputComponent->BindAction("ToggleMainMenu", EInputEvent::IE_Pressed, this, &AXyzPlayerController::ToggleMainMenu);
	ToggleMenuBinding.bExecuteWhenPaused = true;
	InputComponent->BindAction("InteractWithObject", EInputEvent::IE_Pressed, this, &AXyzPlayerController::InteractWithObject);
	InputComponent->BindAction("UseInventory", EInputEvent::IE_Pressed, this, &AXyzPlayerController::UseInventory);
	InputComponent->BindAction("UseRadialMenu", EInputEvent::IE_Pressed, this, &AXyzPlayerController::UseRadialMenu);
	InputComponent->BindAction("QuickSaveGame", EInputEvent::IE_Pressed, this, &AXyzPlayerController::QuickSaveGame);
	InputComponent->BindAction("QuickLoadGame", EInputEvent::IE_Pressed, this, &AXyzPlayerController::QuickLoadGame);

	InputComponent->BindAxis("TurnAtRate", this, &AXyzPlayerController::TurnAtRate);
	InputComponent->BindAxis("LookUpAtRate", this, &AXyzPlayerController::LookUpAtRate);
}

void AXyzPlayerController::CreateAndInitializeHUDWidgets()
{
	if (!IsValid(MainMenuWidget))
	{
		MainMenuWidget = CreateWidget<UUserWidget>(GetWorld(), MainMenuWidgetClass);
	}

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
				CharacterAttributesComponent->OnHealthChangedEvent.AddUFunction(CharacterAttributesWidget, FName("OnHealthChanged"));
				CharacterAttributesComponent->OnStaminaChangedEvent.AddUFunction(CharacterAttributesWidget, FName("OnStaminaChanged"));
				CharacterAttributesComponent->OnOxygenChangedEvent.AddUFunction(CharacterAttributesWidget, FName("OnOxygenChanged"));
			}
		}

		UCharacterAttributesWidget* CharacterAttributesCenterWidget = PlayerHUDWidget->GetCharacterAttributesCenterWidget();
		if (IsValid(CharacterAttributesCenterWidget))
		{
			UCharacterAttributesComponent* CharacterAttributesComponent = CachedBaseCharacter->GetCharacterAttributesComponent();
			if (IsValid(CharacterAttributesComponent))
			{
				CharacterAttributesComponent->OnHealthChangedEvent.AddUFunction(CharacterAttributesCenterWidget, FName("OnHealthChanged"));
				CharacterAttributesComponent->OnStaminaChangedEvent.AddUFunction(CharacterAttributesCenterWidget, FName("OnStaminaChanged"));
				CharacterAttributesComponent->OnOxygenChangedEvent.AddUFunction(CharacterAttributesCenterWidget, FName("OnOxygenChanged"));
			}
		}
	}
	SetInputMode(FInputModeGameOnly{});
	bShowMouseCursor = false;
}

void AXyzPlayerController::ToggleMainMenu()
{
	if (!IsValid(MainMenuWidget) || !IsValid(PlayerHUDWidget))
	{
		return;
	}

	if (MainMenuWidget->IsVisible())
	{
		MainMenuWidget->RemoveFromParent();
		PlayerHUDWidget->AddToViewport();
		SetInputMode(FInputModeGameOnly{});
		SetPause(false);
		bShowMouseCursor = false;
	}
	else
	{
		MainMenuWidget->AddToViewport();
		PlayerHUDWidget->RemoveFromParent();
		SetInputMode(FInputModeGameAndUI{});
		SetPause(true);
		bShowMouseCursor = true;
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
	USaveSubsystem* SaveSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<USaveSubsystem>();
	SaveSubsystem->GetGameSaveDataMutable().bIsSerialized = false; // force disable loading saved data
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

void AXyzPlayerController::InteractWithObject()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->InteractWithObject();
	}
}

void AXyzPlayerController::UseInventory()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->UseInventory(this);
	}
}

void AXyzPlayerController::UseRadialMenu()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->UseRadialMenu(this);
	}
}

void AXyzPlayerController::QuickSaveGame()
{
	 USaveSubsystem* SaveSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<USaveSubsystem>();
	 SaveSubsystem->SaveGame();
}

void AXyzPlayerController::QuickLoadGame()
{
	USaveSubsystem* SaveSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<USaveSubsystem>();
	SaveSubsystem->LoadLastGame();
}

void AXyzPlayerController::OnInteractableObjectFound(FName ActionName)
{
	if (!IsValid(PlayerHUDWidget))
	{
		return;
	}

	TArray<FInputActionKeyMapping> ActionKeys = PlayerInput->GetKeysForAction(ActionName);
	const bool HasAnyKeys = ActionKeys.Num() != 0;
	if (HasAnyKeys)
	{
		FName ActionKey = ActionKeys[0].Key.GetFName();
		PlayerHUDWidget->SetInteractableKeyText(ActionKey);
	}
	PlayerHUDWidget->ShowInteractableKey(HasAnyKeys);
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
