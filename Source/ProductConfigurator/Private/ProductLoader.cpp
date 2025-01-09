// Fill out your copyright notice in the Description page of Project Settings.


#include "ProductLoader.h"
#include "ConfiguratorGameMode.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"

AProductLoader::AProductLoader()
{
	
}

void AProductLoader::BeginPlay()
{
	Super::BeginPlay();

	// Set Reference in Game mode
	if (AConfiguratorGameMode* GameMode = Cast<AConfiguratorGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GameMode->SetProductLoader(this);
		Initialize();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(0, 5, FColor::Red, "GameMode Not Found. Configurator won't work properly.");
	}
}

bool AProductLoader::LoadAssetAsync(FName ProductName, int32 ConfigID)
{
	static const FString ContextString(TEXT("ConfigData"));
	const auto ConfigData = ConfigurationData->FindRow<FConfigurationData>(ProductName, ContextString, true);

	if (ConfigData && ConfigData->Configurations[ConfigID].Asset)
	{
		AsyncAsset = ConfigData->Configurations[ConfigID].Asset;
		if (AsyncAsset)
		{
			FStreamableManager StreamableManager;
			TSharedPtr<FStreamableHandle> Handle = StreamableManager.RequestAsyncLoad(
			AsyncAsset.ToSoftObjectPath(),
			FStreamableDelegate::CreateUObject(this, &AProductLoader::OnAssetLoaded),
			0,
			false
			);

			if (Handle.IsValid()) return true;
		}
	}
	return false;
}

void AProductLoader::OnAssetLoaded()
{
	if (AsyncAsset)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Load Complete"));
		GetStaticMeshComponent()->SetStaticMesh(AsyncAsset.Get());
	}
}

void AProductLoader::Initialize()
{
	if (LoadAssetAsync(DefaultProductName, 0))
	{
		PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PlayerController && ConfigUI)
		{
			EnableInput(PlayerController);
			FInputModeGameAndUI Input;
			Input.SetWidgetToFocus(ConfigUI->TakeWidget());
			PlayerController->SetInputMode(Input);
			PlayerController->SetShowMouseCursor(true);
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Something went wrong."));
	}
}
