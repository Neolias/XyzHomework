// Fill out your copyright notice in the Description page of Project Settings.


#include "JoinSessionWidget.h"

#include "XyzGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UJoinSessionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld());
	check(GameInstance->IsA<UXyzGameInstance>());
	XyzGameInstance = StaticCast<UXyzGameInstance*>(GameInstance);
}

void UJoinSessionWidget::FindOnlineSession()
{
	XyzGameInstance->OnMatchFound.AddUFunction(this, FName("OnMatchFound"));
	XyzGameInstance->FindMatch(bIsLAN);
	SearchingSessionState = ESearchingSessionState::Searching;
}

void UJoinSessionWidget::JoinOnlineSession()
{
	XyzGameInstance->JoinOnlineGame();
}

void UJoinSessionWidget::OnMatchFound_Implementation(bool bIsSuccess)
{
	SearchingSessionState = bIsSuccess ? ESearchingSessionState::SessionIsFound : ESearchingSessionState::None;
	XyzGameInstance->OnMatchFound.RemoveAll(this);
}

void UJoinSessionWidget::CloseWidget()
{
	XyzGameInstance->OnMatchFound.RemoveAll(this);
	Super::CloseWidget();
}
