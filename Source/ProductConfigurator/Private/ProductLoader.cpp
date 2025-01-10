// Fill out your copyright notice in the Description page of Project Settings.


#include "ProductLoader.h"
#include "ConfiguratorGameMode.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/ConfiguratorUI.h"

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

void AProductLoader::LoadAssetAsync(FName ProductName, int32 VariantIndex, int32 VariantSizeIndex, int32 MaterialIndex)
{
	if (!ConfigurationData) return;
	static const FString ContextString(TEXT("ConfigData"));
	auto ConfigData = ConfigurationData->FindRow<FConfigurationData>(ProductName, ContextString, true);
	if (!ConfigData) return;
	
	AsyncAsset = ConfigData->Configurations[VariantIndex].Asset;
	if (AsyncAsset)
	{
		FStreamableManager StreamableManager;
		TSharedPtr<FStreamableHandle> Handle = StreamableManager.RequestAsyncLoad(
		AsyncAsset.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &AProductLoader::OnAssetLoaded),
		0,
		false
		);

		// This Material setup is not ideal but works for now.
		if (MaterialIndex)
		{
			CurrentMaterialOption = MaterialOption2;
		}
		else
		{
			CurrentMaterialOption = MaterialOption1;
		}
	}
}

void AProductLoader::OnAssetLoaded()
{
	if (AsyncAsset)
	{
		GetStaticMeshComponent()->SetStaticMesh(AsyncAsset.Get());
		GetStaticMeshComponent()->SetMaterial(0, CurrentMaterialOption);
	}
}

void AProductLoader::Initialize()
{
	LoadAssetAsync(DefaultProductName, 0);

	PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PlayerController && ConfigurationWidget)
	{
		ConfigUI = CreateWidget<UConfiguratorUI>(GetWorld(), ConfigurationWidget);
		if (ConfigUI)
		{
			ConfigUI->SetLoader(this);
			ConfigUI->AddToViewport();
			EnableInput(PlayerController);
			FInputModeGameAndUI Input;
			Input.SetWidgetToFocus(ConfigUI->TakeWidget());
			PlayerController->SetInputMode(Input);
			PlayerController->SetShowMouseCursor(true);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ProductLoader: Can't create widget."));
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ProductLoader: No Input Widget Found."));
	}
}