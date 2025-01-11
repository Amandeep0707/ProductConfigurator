// Fill out your copyright notice in the Description page of Project Settings.


#include "ConfiguratorGameMode.h"

#include "GameFramework/GameUserSettings.h"


void AConfiguratorGameMode::BeginDestroy()
{
	Super::BeginDestroy();

	GameUserSettings = UGameUserSettings::GetGameUserSettings();
	GameUserSettings->LoadSettings();
	GameUserSettings->ApplySettings(false);
}
