// Fill out your copyright notice in the Description page of Project Settings.


#include "ProductLoader.h"
#include "ConfiguratorGameMode.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/ConfiguratorUI.h"

AProductLoader::AProductLoader()
{
	PrimaryActorTick.bCanEverTick = false;

	GetStaticMeshComponent()->SetRenderCustomDepth(true);
	
	OptionOneComp = CreateDefaultSubobject<UStaticMeshComponent>("OptionOneComponent");
	OptionOneComp->SetupAttachment(GetRootComponent());
	OptionOneComp->SetRenderCustomDepth(true);

	OptionTwoComp = CreateDefaultSubobject<UStaticMeshComponent>("OptionTwoComponent");
	OptionTwoComp->SetupAttachment(GetRootComponent());
	OptionTwoComp->SetRenderCustomDepth(true);
	
	OptionThreeComp = CreateDefaultSubobject<UStaticMeshComponent>("OptionThreeComponent");
	OptionThreeComp->SetupAttachment(GetRootComponent());
	OptionThreeComp->SetRenderCustomDepth(true);
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

void AProductLoader::LoadAssetAsync(FName ProductName, int32 VariantIndex, int32 VariantSizeIndex, int32 MaterialIndex, bool bOptionOne, bool bOptionTwo, bool bOptionThree)
{
	// Check if the current load request is the same as the last load
	bool bSameModelRequested = (ProductName == LastLoadedProductName) && 
								(VariantIndex == LastLoadedVariantIndex) && 
								(VariantSizeIndex == LastLoadedVariantSizeIndex);

	// Only reset toggle state if a new model is being loaded
	if (!bSameModelRequested)
	{
		// Reset toggle state only for a new model
		bCurrentToggleState = false;

		// Update last loaded product details
		LastLoadedProductName = ProductName;
		LastLoadedVariantIndex = VariantIndex;
		LastLoadedVariantSizeIndex = VariantSizeIndex;
	}
	
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
	AsyncOptionOneMesh = AssetDetails.OptionOneAsset;
	AsyncOptionTwoMesh = AssetDetails.OptionTwoAsset;
	AsyncOptionThreeMesh = AssetDetails.OptionThreeAsset;
	AsyncOptionThreeMesh2 = AssetDetails.OptionThreeAsset2;

	if (!AsyncAsset.IsNull() || !AsyncOptionTwoMesh.IsNull() || !AsyncOptionOneMesh.IsNull() || !AsyncOptionThreeMesh.IsNull())
	{
		TArray<FSoftObjectPath> AssetPaths;
		AssetPaths.Add(AsyncAsset.ToSoftObjectPath());
		
		if (bOptionOne)
		{
			AssetPaths.Add(AsyncOptionOneMesh.ToSoftObjectPath());
			bOptionOneVisible = bOptionOne;
		} else bOptionOneVisible = false;
		
		if (bOptionTwo)
		{
			AssetPaths.Add(AsyncOptionTwoMesh.ToSoftObjectPath());
			bOptionTwoVisible = bOptionTwo;
		} else bOptionTwoVisible = false;
		
		// Option Three - Enhanced Toggle Handling
		AsyncOptionThreeMesh = AssetDetails.OptionThreeAsset;
		AsyncOptionThreeMesh2 = AssetDetails.OptionThreeAsset2;
		bOptionThreeVisible = false;

		// Check if toggle is supported and both meshes are valid
		if (AssetDetails.bUseOptionThreeAsToggle && 
		!AsyncOptionThreeMesh.IsNull() && 
		!AsyncOptionThreeMesh2.IsNull())
		{
			bUseToggleForOptionThree = true;
			bOptionThreeVisible = true;
			bUseOptionThreeMaterialSelector = AssetDetails.bUseMaterialSelector;

			// Add both toggle meshes to loading
			AssetPaths.Add(AsyncOptionThreeMesh.ToSoftObjectPath());
			AssetPaths.Add(AsyncOptionThreeMesh2.ToSoftObjectPath());
		}
		else if (bOptionThree && !AsyncOptionThreeMesh.IsNull())
		{
			// Standard Option Three behavior
			bOptionThreeVisible = true;
			bUseOptionThreeMaterialSelector = AssetDetails.bUseMaterialSelector;
			AssetPaths.Add(AsyncOptionThreeMesh.ToSoftObjectPath());
		}

		// Material selection
		CurrentMaterialOption = (MaterialIndex != 0) ? MaterialOption2 : MaterialOption1;

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
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Some assets are not set properly in the configuration."));
	}
}

void AProductLoader::OnAssetLoaded()
{
	if (AsyncAsset.IsValid())
	{
		GetStaticMeshComponent()->SetStaticMesh(AsyncAsset.Get());
		GetStaticMeshComponent()->SetMaterial(MaterialSelectorIndex, CurrentMaterialOption);
	}
	if (AsyncOptionOneMesh.IsValid())
	{
		OptionOneComp->SetStaticMesh(AsyncOptionOneMesh.Get());
		OptionOneComp->SetVisibility(bOptionOneVisible);
		OptionOneComp->SetMaterial(MaterialSelectorIndex, CurrentMaterialOption);
	}
	else OptionOneComp->SetVisibility(false);
	
	if (AsyncOptionTwoMesh.IsValid())
	{
		OptionTwoComp->SetStaticMesh(AsyncOptionTwoMesh.Get());
		OptionTwoComp->SetVisibility(bOptionTwoVisible);
		OptionTwoComp->SetMaterial(MaterialSelectorIndex, CurrentMaterialOption);
	}
	else OptionTwoComp->SetVisibility(false);
	
	// Option Three - Toggle or Standard
	if (bUseToggleForOptionThree)
	{
		// Ensure both toggle meshes are valid
		if (AsyncOptionThreeMesh.IsValid() && AsyncOptionThreeMesh2.IsValid())
		{
			OptionThreeComp->SetVisibility(true);

			// Select mesh based on current toggle state
			TSoftObjectPtr<UStaticMesh> CurrentToggleMesh = 
				bCurrentToggleState ? AsyncOptionThreeMesh2 : AsyncOptionThreeMesh;

			OptionThreeComp->SetStaticMesh(CurrentToggleMesh.Get());

			// Material handling
			if (bUseOptionThreeMaterialSelector)
			{
				OptionThreeComp->SetMaterial(MaterialSelectorIndex, CurrentMaterialOption);
			}
			else
			{
				// Use the default material of the current mesh
				OptionThreeComp->SetMaterial(
					MaterialSelectorIndex, 
					CurrentToggleMesh.Get()->GetMaterial(MaterialSelectorIndex)
				);
			}

			// Toggle state for next time
			bCurrentToggleState = !bCurrentToggleState;
		}
		else
		{
			OptionThreeComp->SetVisibility(false);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, 
				TEXT("Toggle meshes for Option Three are not properly configured."));
		}
	}
	else if (AsyncOptionThreeMesh.IsValid())
	{
		// Standard Option Three behavior
		OptionThreeComp->SetStaticMesh(AsyncOptionThreeMesh.Get());
		OptionThreeComp->SetVisibility(bOptionThreeVisible);

		if (bUseOptionThreeMaterialSelector)
		{
			OptionThreeComp->SetMaterial(MaterialSelectorIndex, CurrentMaterialOption);
		}
		else
		{
			OptionThreeComp->SetMaterial(
				MaterialSelectorIndex, 
				AsyncOptionThreeMesh.Get()->GetMaterial(MaterialSelectorIndex)
			);
		}
	}
	else
	{
		OptionThreeComp->SetVisibility(false);
	}
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
	OptionOneComp->SetRenderCustomDepth(true);
	OptionTwoComp->SetRenderCustomDepth(true);
	OptionThreeComp->SetRenderCustomDepth(true);
}

void AProductLoader::OnMouseExitMesh()
{
	GetStaticMeshComponent()->SetRenderCustomDepth(false);
	OptionOneComp->SetRenderCustomDepth(false);
	OptionTwoComp->SetRenderCustomDepth(false);
	OptionThreeComp->SetRenderCustomDepth(false);
}