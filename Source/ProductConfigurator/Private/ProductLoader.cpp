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
    GetStaticMeshComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

    // Create Rooftop Component
    RooftopComp = CreateDefaultSubobject<UStaticMeshComponent>("RooftopComponent");
    RooftopComp->SetupAttachment(GetRootComponent());
    RooftopComp->SetRenderCustomDepth(true);
    RooftopComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    
    // Create Side Options Component
    SideOptionComp = CreateDefaultSubobject<UStaticMeshComponent>("SideOptionComponent");
    SideOptionComp->SetupAttachment(GetRootComponent());
    SideOptionComp->SetRenderCustomDepth(true);
    SideOptionComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    
    // Create Front Options Component
    FrontOptionComp = CreateDefaultSubobject<UStaticMeshComponent>("FrontOptionComponent");
    FrontOptionComp->SetupAttachment(GetRootComponent());
    FrontOptionComp->SetRenderCustomDepth(true);
    FrontOptionComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

    // Create Additional Mesh Component
    AdditionalMeshComp = CreateDefaultSubobject<UStaticMeshComponent>("AdditionalMeshComponent");
    AdditionalMeshComp->SetupAttachment(GetRootComponent());
    AdditionalMeshComp->SetRenderCustomDepth(true);
    AdditionalMeshComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
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

void AProductLoader::LoadAssetAsync(FName ProductName, int32 VariantIndex, int32 VariantSizeIndex, int32 MaterialIndex, ESideOption SideOption, EFrontOption FrontOption)
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

    // Get the correct asset details
    const FAssetDetails& AssetDetails = Configuration.Assets[VariantSizeIndex];

    // Check if the newly loaded model is different from the last one
    if (LastLoadedProduct != ProductName || LastVariantIndex != VariantIndex || LastVariantSizeIndex != VariantSizeIndex)
    {
        // Reset toggle state for a new model
        bCurrentRooftopToggleState = false;
    }
    
    // Store rooftop options
    bRooftopToggleAvailable = AssetDetails.bUseRooftopToggle;
    AsyncRooftopAsset1 = AssetDetails.RooftopAsset1;
    AsyncRooftopAsset2 = AssetDetails.RooftopAsset2;
    
    // Store assets to be loaded
    AsyncAsset = AssetDetails.Asset;
    
    // Store the currently selected options
    CurrentSideOption = SideOption;
    CurrentFrontOption = FrontOption;
    
    // Store side option meshes
    AsyncSideGlassOptionAsset = AssetDetails.SideGlassOptionAsset;
    AsyncSideHalfFencingOptionAsset = AssetDetails.SideHalfFencingOptionAsset;
    AsyncSideFullFencingOptionAsset = AssetDetails.SideFullFencingOptionAsset;
    
    // Store front option meshes
    AsyncFrontGlassOptionAsset = AssetDetails.FrontGlassOptionAsset;
    AsyncFrontFullFencingOptionAsset = AssetDetails.FrontFullFencingOptionAsset;

    // Store additional mesh
    if (AssetDetails.bLoadAdditionalMesh)
    {
        AsyncAdditionalMeshAsset = AssetDetails.AdditionalMeshAsset;
    }
    else
    {
        // Clear the pointer if the new asset doesn't use an additional mesh
        AsyncAdditionalMeshAsset = nullptr;
    }
    
    // Initialize loading paths
    TArray<FSoftObjectPath> AssetPaths;
    if (!AsyncAsset.IsNull())
    {
        AssetPaths.Add(AsyncAsset.ToSoftObjectPath());
    }

    // Add rooftop assets to load
    if (!AsyncRooftopAsset1.IsNull())
    {
        AssetPaths.Add(AsyncRooftopAsset1.ToSoftObjectPath());
    }
    if (bRooftopToggleAvailable && !AsyncRooftopAsset2.IsNull())
    {
        AssetPaths.Add(AsyncRooftopAsset2.ToSoftObjectPath());
    }
    
    // Add side option asset to load
    if (SideOption != ESideOption::None && AssetDetails.bSideOptionsAvailable)
    {
        switch (SideOption)
        {
            case ESideOption::Glass:
                if (!AsyncSideGlassOptionAsset.IsNull())
                    AssetPaths.Add(AsyncSideGlassOptionAsset.ToSoftObjectPath());
                break;
            case ESideOption::HalfFencing:
                if (!AsyncSideHalfFencingOptionAsset.IsNull())
                    AssetPaths.Add(AsyncSideHalfFencingOptionAsset.ToSoftObjectPath());
                break;
            case ESideOption::FullFencing:
                if (!AsyncSideFullFencingOptionAsset.IsNull())
                    AssetPaths.Add(AsyncSideFullFencingOptionAsset.ToSoftObjectPath());
                break;
        }
    }
    
    // Add front option asset to load
    if (FrontOption != EFrontOption::None && AssetDetails.bFrontOptionsAvailable)
    {
        switch (FrontOption)
        {
            case EFrontOption::Glass:
                if (!AsyncFrontGlassOptionAsset.IsNull())
                    AssetPaths.Add(AsyncFrontGlassOptionAsset.ToSoftObjectPath());
                break;
            case EFrontOption::FullFencing:
                if (!AsyncFrontFullFencingOptionAsset.IsNull())
                    AssetPaths.Add(AsyncFrontFullFencingOptionAsset.ToSoftObjectPath());
                break;
        }
    }

    // Add additional mesh asset to load
    if (AssetDetails.bLoadAdditionalMesh && !AsyncAdditionalMeshAsset.IsNull())
    {
        AssetPaths.Add(AsyncAdditionalMeshAsset.ToSoftObjectPath());
    }

    // Material selection
    CurrentMaterialOption = (MaterialIndex != 0) ? MaterialOption2 : MaterialOption1;

    // Store current model info
    LastLoadedProduct = ProductName;
    LastVariantIndex = VariantIndex;
    LastVariantSizeIndex = VariantSizeIndex;

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

void AProductLoader::OnAssetLoaded()
{
    // Set base mesh
    if (AsyncAsset.IsValid())
    {
        GetStaticMeshComponent()->SetStaticMesh(AsyncAsset.Get());
        GetStaticMeshComponent()->SetMaterial(MaterialSelectorIndex, CurrentMaterialOption);
    }

    // Update rooftop option mesh based on current selection
    UpdateRooftopMesh();
    
    // Update side option mesh based on current selection
    UpdateSideOptionMesh();
    
    // Update front option mesh based on current selection
    UpdateFrontOptionMesh();

    // Update additional mesh
    UpdateAdditionalMesh();
    
    OnProductLoaded.Execute();
}

void AProductLoader::UpdateSideOptionMesh()
{
    // Handle side option visibility and mesh based on current selection
    if (CurrentSideOption == ESideOption::None)
    {
        SideOptionComp->SetVisibility(false);
        return;
    }
    
    TSoftObjectPtr<UStaticMesh> SelectedMesh = nullptr;
    bool bValidMesh = false;
    
    switch (CurrentSideOption)
    {
        case ESideOption::Glass:
            if (AsyncSideGlassOptionAsset.IsValid())
            {
                SelectedMesh = AsyncSideGlassOptionAsset;
                bValidMesh = true;
            }
            break;
        case ESideOption::HalfFencing:
            if (AsyncSideHalfFencingOptionAsset.IsValid())
            {
                SelectedMesh = AsyncSideHalfFencingOptionAsset;
                bValidMesh = true;
            }
            break;
        case ESideOption::FullFencing:
            if (AsyncSideFullFencingOptionAsset.IsValid())
            {
                SelectedMesh = AsyncSideFullFencingOptionAsset;
                bValidMesh = true;
            }
            break;
    }
    
    if (bValidMesh)
    {
        SideOptionComp->SetStaticMesh(SelectedMesh.Get());
        SideOptionComp->SetVisibility(true);
        SideOptionComp->SetMaterial(MaterialSelectorIndex, CurrentMaterialOption);
    }
    else
    {
        SideOptionComp->SetVisibility(false);
    }
}

void AProductLoader::UpdateFrontOptionMesh()
{
    // Handle front option visibility and mesh based on current selection
    if (CurrentFrontOption == EFrontOption::None)
    {
        FrontOptionComp->SetVisibility(false);
        return;
    }
    
    TSoftObjectPtr<UStaticMesh> SelectedMesh = nullptr;
    bool bValidMesh = false;
    
    switch (CurrentFrontOption)
    {
        case EFrontOption::Glass:
            if (AsyncFrontGlassOptionAsset.IsValid())
            {
                SelectedMesh = AsyncFrontGlassOptionAsset;
                bValidMesh = true;
            }
            break;
        case EFrontOption::FullFencing:
            if (AsyncFrontFullFencingOptionAsset.IsValid())
            {
                SelectedMesh = AsyncFrontFullFencingOptionAsset;
                bValidMesh = true;
            }
            break;
    }
    
    if (bValidMesh)
    {
        FrontOptionComp->SetStaticMesh(SelectedMesh.Get());
        FrontOptionComp->SetVisibility(true);
        FrontOptionComp->SetMaterial(MaterialSelectorIndex, CurrentMaterialOption);
    }
    else
    {
        FrontOptionComp->SetVisibility(false);
    }
}

void AProductLoader::UpdateAdditionalMesh()
{
    if (AsyncAdditionalMeshAsset.IsValid())
    {
        AdditionalMeshComp->SetStaticMesh(AsyncAdditionalMeshAsset.Get());
        AdditionalMeshComp->SetVisibility(true);
        AdditionalMeshComp->SetMaterial(MaterialSelectorIndex, CurrentMaterialOption);
    }
    else
    {
        // Hide the component if the asset is not valid or not used
        AdditionalMeshComp->SetStaticMesh(nullptr);
        AdditionalMeshComp->SetVisibility(false);
    }
}

void AProductLoader::SetSideOption(ESideOption Option)
{
    // Only update if the option has changed
    if (CurrentSideOption != Option)
    {
        CurrentSideOption = Option;
        // The mesh is already loaded, we just need to set it
        UpdateSideOptionMesh();
    }
}

void AProductLoader::SetFrontOption(EFrontOption Option)
{
    // Only update if the option has changed
    if (CurrentFrontOption != Option)
    {
        CurrentFrontOption = Option;
        // The mesh is already loaded, we just need to set it
        UpdateFrontOptionMesh();
    }
}

void AProductLoader::ToggleRooftop()
{
    if (!bRooftopToggleAvailable)
    {
        // Optional: Log a warning if called on a non-toggleable product
        GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("Rooftop toggle not available for this product."));
        return;
    }

    // Flip the toggle state
    bCurrentRooftopToggleState = !bCurrentRooftopToggleState;

    // Update the mesh
    UpdateRooftopMesh();
}

void AProductLoader::Initialize()
{
    // Initialize with default product and options
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

void AProductLoader::UpdateRooftopMesh()
{
    TSoftObjectPtr<UStaticMesh> SelectedRooftopMesh = nullptr;

    if (bRooftopToggleAvailable)
    {
        // Select mesh based on toggle state
        SelectedRooftopMesh = bCurrentRooftopToggleState ? AsyncRooftopAsset2 : AsyncRooftopAsset1;
    }
    else
    {
        // Use the default single rooftop mesh
        SelectedRooftopMesh = AsyncRooftopAsset1;
    }

    if (SelectedRooftopMesh.IsValid())
    {
        RooftopComp->SetStaticMesh(SelectedRooftopMesh.Get());
        RooftopComp->SetVisibility(true);
        RooftopComp->SetMaterial(MaterialSelectorIndex, CurrentMaterialOption);
    }
    else
    {
        RooftopComp->SetVisibility(false);
    }
}

void AProductLoader::OnMouseOverMesh()
{
    GetStaticMeshComponent()->SetRenderCustomDepth(true);
    RooftopComp->SetRenderCustomDepth(true);
    SideOptionComp->SetRenderCustomDepth(true);
    FrontOptionComp->SetRenderCustomDepth(true);
    AdditionalMeshComp->SetRenderCustomDepth(true);
}

void AProductLoader::OnMouseExitMesh()
{
    GetStaticMeshComponent()->SetRenderCustomDepth(false);
    RooftopComp->SetRenderCustomDepth(false);
    SideOptionComp->SetRenderCustomDepth(false);
    FrontOptionComp->SetRenderCustomDepth(false);
    AdditionalMeshComp->SetRenderCustomDepth(false);
}