// Fill out your copyright notice in the Description page of Project Settings.


#include "ProductLoader.h"
#include "ConfiguratorGameMode.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/ConfiguratorUI.h"

AProductLoader::AProductLoader()
{
	PrimaryActorTick.bCanEverTick = true;

	GetStaticMeshComponent()->SetRenderCustomDepth(true);
	
	HalfFencingMeshComp = CreateDefaultSubobject<UStaticMeshComponent>("HalfFencingMeshComponent");
	HalfFencingMeshComp->SetupAttachment(GetRootComponent());
	HalfFencingMeshComp->SetRenderCustomDepth(true);

	FullFencingMeshComp = CreateDefaultSubobject<UStaticMeshComponent>("FullFencingMeshComponent");
	FullFencingMeshComp->SetupAttachment(GetRootComponent());
	FullFencingMeshComp->SetRenderCustomDepth(true);
	
	GlassMeshComp = CreateDefaultSubobject<UStaticMeshComponent>("GlassMeshComponent");
	GlassMeshComp->SetupAttachment(GetRootComponent());
	GlassMeshComp->SetRenderCustomDepth(true);
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

void AProductLoader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProductLoader::LoadAssetAsync(FName ProductName, int32 VariantIndex, int32 VariantSizeIndex, int32 MaterialIndex, bool bHalfFencing, bool bFullFencing, bool bGlassOption)
{
	if (!ConfigurationData)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Configuration Data is null!"));
		return;
	}

	static const FString ContextString(TEXT("ConfigData"));
	FConfigurationData* ConfigData = ConfigurationData->FindRow<FConfigurationData>(ProductName, ContextString, true);
    
	if (!ConfigData)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Product not found: %s. Please set correct DefaultProductName in Product Loader."), *ProductName.ToString()));
		return;
	}

	// Validate indices
	if (!ConfigData->Configurations.IsValidIndex(VariantIndex))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Invalid variant index: %d"), VariantIndex));
		return;
	}

	const FConfigurationDetails& Configuration = ConfigData->Configurations[VariantIndex];
    
	if (!Configuration.Assets.IsValidIndex(VariantSizeIndex))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Invalid size index: %d"), VariantSizeIndex));
		return;
	}

	// Get the correct asset
	const FAssetDetails& AssetDetails = Configuration.Assets[VariantSizeIndex];
	AsyncAsset = AssetDetails.Asset;
	AsyncHalfFencingMesh = AssetDetails.HalfFencingAsset;
	AsyncFullFencingMesh = AssetDetails.FullFencingAsset;
	AsyncGlassMesh = AssetDetails.GlassAsset;

	if (!AsyncAsset.IsNull())
	{
		TArray<FSoftObjectPath> AssetPaths;
		AssetPaths.Add(AsyncAsset.ToSoftObjectPath());
		
		if (bHalfFencing)
		{
			AssetPaths.Add(AsyncHalfFencingMesh.ToSoftObjectPath());
			bHalfFencingVisible = bHalfFencing;
		} else bHalfFencingVisible = false;
		
		if (bFullFencing)
		{
			AssetPaths.Add(AsyncFullFencingMesh.ToSoftObjectPath());
			bFullFencingVisible = bFullFencing;
		} else bFullFencingVisible = false;
		
		if (bGlassOption)
		{
			AssetPaths.Add(AsyncGlassMesh.ToSoftObjectPath());
			bGlassVisible = bGlassOption;
		} else bGlassVisible = false;

		// Create a persistent StreamableManager instance
		if (!StreamableManager)
		{
			StreamableManager = MakeShared<FStreamableManager>();
		}

		// Store the handle as a class member
		StreamableHandle = StreamableManager->RequestAsyncLoad(
			AssetPaths,
			FStreamableDelegate::CreateUObject(this, &AProductLoader::OnAssetLoaded),
			0,
			false
		);

		// Add handle validation
		if (!StreamableHandle.IsValid())
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to create streamable handle"));
			return;
		}

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
	if (AsyncAsset.IsValid())
	{
		GetStaticMeshComponent()->SetStaticMesh(AsyncAsset.Get());
		GetStaticMeshComponent()->SetMaterial(MaterialSelectorIndex, CurrentMaterialOption);
	}
	if (AsyncHalfFencingMesh.IsValid())
	{
		HalfFencingMeshComp->SetStaticMesh(AsyncHalfFencingMesh.Get());
		HalfFencingMeshComp->SetVisibility(bHalfFencingVisible);
		HalfFencingMeshComp->SetMaterial(MaterialSelectorIndex, CurrentMaterialOption);
	}
	else HalfFencingMeshComp->SetVisibility(false);
	
	if (AsyncFullFencingMesh.IsValid())
	{
		FullFencingMeshComp->SetStaticMesh(AsyncFullFencingMesh.Get());
		FullFencingMeshComp->SetVisibility(bFullFencingVisible);
		FullFencingMeshComp->SetMaterial(MaterialSelectorIndex, CurrentMaterialOption);
	}
	else FullFencingMeshComp->SetVisibility(false);
	
	if (AsyncGlassMesh.IsValid())
	{
		GlassMeshComp->SetStaticMesh(AsyncGlassMesh.Get());
		GlassMeshComp->SetVisibility(bGlassVisible);
	}
	else GlassMeshComp->SetVisibility(false);
}

void AProductLoader::Initialize()
{
	LoadAssetAsync(DefaultProductName);

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

void AProductLoader::OnMouseOverMesh()
{
	GetStaticMeshComponent()->SetRenderCustomDepth(true);
	HalfFencingMeshComp->SetRenderCustomDepth(true);
	FullFencingMeshComp->SetRenderCustomDepth(true);
	GlassMeshComp->SetRenderCustomDepth(true);
}

void AProductLoader::OnMouseExitMesh()
{
	GetStaticMeshComponent()->SetRenderCustomDepth(false);
	HalfFencingMeshComp->SetRenderCustomDepth(false);
	FullFencingMeshComp->SetRenderCustomDepth(false);
	GlassMeshComp->SetRenderCustomDepth(false);
}