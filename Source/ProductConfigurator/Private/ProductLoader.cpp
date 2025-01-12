// Fill out your copyright notice in the Description page of Project Settings.


#include "ProductLoader.h"
#include "ConfiguratorGameMode.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/ConfiguratorUI.h"

AProductLoader::AProductLoader()
{
	PrimaryActorTick.bCanEverTick = true;
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
	TraceUnderMouse();
}

void AProductLoader::LoadAssetAsync(FName ProductName, int32 VariantIndex, int32 VariantSizeIndex, int32 MaterialIndex)
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
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Product not found: %s"), *ProductName.ToString()));
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

	if (!AsyncAsset.IsNull())
	{
		// Create a persistent StreamableManager instance
		if (!StreamableManager)
		{
			StreamableManager = MakeShared<FStreamableManager>();
		}

		// Store the handle as a class member
		StreamableHandle = StreamableManager->RequestAsyncLoad(
			AsyncAsset.ToSoftObjectPath(),
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
		GetStaticMeshComponent()->SetMaterial(0, CurrentMaterialOption);
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
}

void AProductLoader::OnMouseExitMesh()
{
	GetStaticMeshComponent()->SetRenderCustomDepth(false);
}

void AProductLoader::TraceUnderMouse()
{
	if (!PlayerController)
	{
		return;
	}

	// Get mouse position
	float MouseX, MouseY;
	if (!PlayerController->GetMousePosition(MouseX, MouseY))
	{
		if (bIsMouseOver)
		{
			bIsMouseOver = false;
			OnMouseExitMesh();
		}
		return;
	}

	// Convert mouse position to world space
	FVector WorldLocation, WorldDirection;
	if (PlayerController->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection))
	{
		// Calculate end point
		FVector EndLocation = WorldLocation + (WorldDirection * 10000.0f);

		// Setup trace parameters
		FHitResult HitResult;
		TArray<AActor*> ActorsToIgnore;
		
		bool bHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(),
			WorldLocation,
			EndLocation,
			UEngineTypes::ConvertToTraceType(ECC_Visibility),
			false,
			ActorsToIgnore,
			EDrawDebugTrace::None,
			HitResult,
			false
			);

		// Check if we hit our static mesh component
		if (bHit && HitResult.GetComponent() == GetStaticMeshComponent())
		{
			if (!bIsMouseOver)
			{
				bIsMouseOver = true;
			}
			OnMouseOverMesh();
		}
		else if (bIsMouseOver)
		{
			bIsMouseOver = false;
			OnMouseExitMesh();
		}
	}
}
