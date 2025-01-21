// Fill out your copyright notice in the Description page of Project Settings.


#include "HostSessionWidget.h"

#include "XyzGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UHostSessionWidget::CreateSession()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld());
	check(GameInstance->IsA<UXyzGameInstance>());
	UXyzGameInstance* XyzGameInstance = StaticCast<UXyzGameInstance*>(GameInstance);

	XyzGameInstance->LaunchLobby(4, ServerName, bIsLAN);
}
