// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StreamableManager.h"
#include "ProductLoader.generated.h"

DECLARE_DYNAMIC_DELEGATE(FOnProductLoaded);

class UConfiguratorUI;

// Enums for side and front options
UENUM(BlueprintType)
enum class ESideOption : uint8
{
    None        UMETA(DisplayName = "None"),
    Glass       UMETA(DisplayName = "Glass"),
    HalfFencing UMETA(DisplayName = "Half Fencing"),
    FullFencing UMETA(DisplayName = "Full Fencing")
};

UENUM(BlueprintType)
enum class EFrontOption : uint8
{
    None        UMETA(DisplayName = "None"),
    Glass       UMETA(DisplayName = "Glass"),
    FullFencing UMETA(DisplayName = "Full Fencing")
};

USTRUCT(BlueprintType)
struct FAssetDetails : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FName AssetName;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UTexture2D* Thumbnail;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSoftObjectPtr<UStaticMesh> Asset;

    // --- Rooftop Assets ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSoftObjectPtr<UStaticMesh> RooftopAsset1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    bool bUseRooftopToggle;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bUseRooftopToggle", EditConditionHides))
    TSoftObjectPtr<UStaticMesh> RooftopAsset2;
    // --------------------

    // Side Options
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    bool bSideOptionsAvailable;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bSideOptionsAvailable", EditConditionHides))
    FName SideOptionsDisplayName;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bSideOptionsAvailable", EditConditionHides))
    TSoftObjectPtr<UStaticMesh> SideGlassOptionAsset;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bSideOptionsAvailable", EditConditionHides))
    TSoftObjectPtr<UStaticMesh> SideHalfFencingOptionAsset;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bSideOptionsAvailable", EditConditionHides))
    TSoftObjectPtr<UStaticMesh> SideFullFencingOptionAsset;

    // Front Options
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    bool bFrontOptionsAvailable;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bFrontOptionsAvailable", EditConditionHides))
    FName FrontOptionsDisplayName;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bFrontOptionsAvailable", EditConditionHides))
    TSoftObjectPtr<UStaticMesh> FrontGlassOptionAsset;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bFrontOptionsAvailable", EditConditionHides))
    TSoftObjectPtr<UStaticMesh> FrontFullFencingOptionAsset;
    
    // Additional Mesh
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    bool bLoadAdditionalMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bLoadAdditionalMesh", EditConditionHides))
    TSoftObjectPtr<UStaticMesh> AdditionalMeshAsset;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bSideOptionsAvailable || bFrontOptionsAvailable", EditConditionHides))
    bool bUseMaterialSelector = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FVector Dimensions;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float Price = 0.f;

    FAssetDetails()
    {
        AssetName = TEXT("Default");
        Thumbnail = nullptr;
        
        bUseRooftopToggle = false;
        
        // Initialize Side Options
        bSideOptionsAvailable = false;
        SideOptionsDisplayName = TEXT("Side Options");
        
        // Initialize Front Options
        bFrontOptionsAvailable = false;
        FrontOptionsDisplayName = TEXT("Front Options");
        
        bLoadAdditionalMesh = false;
        
        bUseMaterialSelector = false;
        Dimensions = FVector::ZeroVector;
        Price = 0.f;
    }
};

USTRUCT(BlueprintType)
struct FConfigurationDetails : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FName ProductName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TArray<FAssetDetails> Assets;

    FConfigurationDetails()
    {
        ProductName = TEXT("Default");
    }
};

USTRUCT(BlueprintType)
struct FConfigurationData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FName DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UTexture2D* Thumbnail;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TArray<FConfigurationDetails> Configurations;

    FConfigurationData()
    {
        DisplayName = TEXT("Default");
        Thumbnail = nullptr;
    }
};

UENUM()
enum ECurrency
{
    STR UMETA(DisplayName = "Sterling"),
    EUR UMETA(DisplayName = "Euro"),
};

UCLASS()
class AProductLoader : public AStaticMeshActor
{
    GENERATED_BODY()
    AProductLoader();
    virtual void BeginPlay() override;

public:

    FOnProductLoaded OnProductLoaded;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(RowType="ConfigurationData"), Category = Configuration)
    UDataTable* ConfigurationData = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Configuration)
    FName DefaultProductName = TEXT("Bosco");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Configuration)
    TEnumAsByte<ECurrency> Currency = ECurrency::EUR;

    UPROPERTY(EditDefaultsOnly, Category = Configuration)
    TSubclassOf<UUserWidget> ConfigurationWidget;

    UPROPERTY(EditDefaultsOnly, Category = Configuration)
    UMaterialInterface* MaterialOption1 = nullptr;
    
    UPROPERTY(EditDefaultsOnly, Category = Configuration)
    UMaterialInterface* MaterialOption2 = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = Configuration)
    int32 MaterialSelectorIndex = 0;

    FORCEINLINE UConfiguratorUI* GetConfigUI() const { return ConfigUI; }

    UPROPERTY()
    bool bIsMouseOver = false;

    void OnMouseOverMesh();
    void OnMouseExitMesh();

    // New functions for side and front options
    UFUNCTION(BlueprintCallable, Category = Configuration)
    void SetSideOption(ESideOption Option);
    
    UFUNCTION(BlueprintCallable, Category = Configuration)
    void SetFrontOption(EFrontOption Option);

    UFUNCTION(BlueprintCallable, Category = Configuration)
    void ToggleRooftop();

private:
    /**
     * Internal Variables
     */

    // Rooftop Component
    UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    UStaticMeshComponent* RooftopComp = nullptr;

    UPROPERTY()
    bool bRooftopToggleAvailable = false;
    
    UPROPERTY()
    bool bCurrentRooftopToggleState = false;

    // Side Options Component
    UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    UStaticMeshComponent* SideOptionComp = nullptr;

    UPROPERTY()
    ESideOption CurrentSideOption = ESideOption::None;

    // Front Options Component
    UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    UStaticMeshComponent* FrontOptionComp = nullptr;

    UPROPERTY()
    EFrontOption CurrentFrontOption = EFrontOption::None;

    UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    UStaticMeshComponent* AdditionalMeshComp = nullptr;
    
    UPROPERTY()
    FName LastLoadedProduct;
    
    UPROPERTY()
    int32 LastVariantIndex = -1;
    
    UPROPERTY()
    int32 LastVariantSizeIndex = -1;
    
    // Base mesh asset
    UPROPERTY()
    TSoftObjectPtr<UStaticMesh> AsyncAsset;

    // Rooftop Assets
    UPROPERTY()
    TSoftObjectPtr<UStaticMesh> AsyncRooftopAsset1;
    
    UPROPERTY()
    TSoftObjectPtr<UStaticMesh> AsyncRooftopAsset2;
    
    // Side Option assets
    UPROPERTY()
    TSoftObjectPtr<UStaticMesh> AsyncSideGlassOptionAsset;
    
    UPROPERTY()
    TSoftObjectPtr<UStaticMesh> AsyncSideHalfFencingOptionAsset;
    
    UPROPERTY()
    TSoftObjectPtr<UStaticMesh> AsyncSideFullFencingOptionAsset;
    
    // Front Option assets
    UPROPERTY()
    TSoftObjectPtr<UStaticMesh> AsyncFrontGlassOptionAsset;
    
    UPROPERTY()
    TSoftObjectPtr<UStaticMesh> AsyncFrontFullFencingOptionAsset;

    UPROPERTY()
    TSoftObjectPtr<UStaticMesh> AsyncAdditionalMeshAsset;
    
    UPROPERTY()
    APlayerController* PlayerController = nullptr;

    UPROPERTY()
    UConfiguratorUI* ConfigUI = nullptr;

    UPROPERTY()
    UMaterialInterface* CurrentMaterialOption = nullptr;
    
    TSharedPtr<FStreamableManager> StreamableManager;
    TSharedPtr<FStreamableHandle> StreamableHandle;
    
    /**
     * Exposed Functions
     */
    UFUNCTION(BlueprintCallable, Category = Configuration)
    void LoadAssetAsync(FName ProductName, int32 VariantIndex = 0, int32 VariantSizeIndex = 0, int32 MaterialIndex = 0, ESideOption SideOption = ESideOption::None, EFrontOption FrontOption = EFrontOption::None);
    /**
     * Internal Functions
     */
    void OnAssetLoaded();
    void Initialize();
    void UpdateRooftopMesh();
    void UpdateSideOptionMesh();
    void UpdateFrontOptionMesh();
    void UpdateAdditionalMesh();
};