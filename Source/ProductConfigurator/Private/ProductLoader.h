// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StreamableManager.h"
#include "ProductLoader.generated.h"

DECLARE_DYNAMIC_DELEGATE(FOnProductLoaded);

class UConfiguratorUI;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bOptionOneAvailable;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bOptionOneAvailable", EditConditionHides))
	FName OptionOneDisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bOptionOneAvailable", EditConditionHides))
	TSoftObjectPtr<UStaticMesh> OptionOneAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bOptionTwoAvailable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bOptionTwoAvailable", EditConditionHides))
	FName OptionTwoDisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bOptionTwoAvailable", EditConditionHides))
	TSoftObjectPtr<UStaticMesh> OptionTwoAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bOptionThreeAvailable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bOptionThreeAvailable", EditConditionHides))
	bool bUseOptionThreeAsDefault;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bOptionThreeAvailable", EditConditionHides))
	FName OptionThreeDisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bOptionThreeAvailable", EditConditionHides))
	TSoftObjectPtr<UStaticMesh> OptionThreeAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bOptionThreeAvailable", EditConditionHides))
	bool bUseOptionThreeAsToggle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bUseOptionThreeAsToggle", EditConditionHides))
	TSoftObjectPtr<UStaticMesh> OptionThreeAsset2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bOptionThreeAvailable", EditConditionHides))
	bool bUseMaterialSelector = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector Dimensions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Price = 0.f;

	FAssetDetails()
	{
		AssetName = TEXT("Default");
		Thumbnail = nullptr;
		bOptionOneAvailable = false;
		OptionOneDisplayName = TEXT("Half Fencing");
		bOptionTwoAvailable = false;
		OptionTwoDisplayName = TEXT("Full Fencing");
		bOptionThreeAvailable = false;
		bUseOptionThreeAsDefault = true;
		OptionThreeDisplayName = TEXT("Glass");
		bUseOptionThreeAsToggle = false;
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

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = Configuration)
	FName DefaultProductName = TEXT("Default");

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

private:

	/**
	 * Internal Variables
	 */

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	UStaticMeshComponent* OptionOneComp = nullptr;

	UPROPERTY()
	bool bOptionOneVisible = false;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	UStaticMeshComponent* OptionTwoComp = nullptr;

	UPROPERTY()
	bool bOptionTwoVisible = false;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	UStaticMeshComponent* OptionThreeComp = nullptr;

	UPROPERTY()
	bool bUseOptionThreeMaterialSelector = true;

	UPROPERTY()
	bool bOptionThreeVisible = false;

	UPROPERTY()
	bool bUseToggleForOptionThree = false;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	bool bCurrentToggleState = false;

	UPROPERTY()
	FName LastLoadedProduct;
    
	UPROPERTY()
	int32 LastVariantIndex = -1;
    
	UPROPERTY()
	int32 LastVariantSizeIndex = -1;
	
	UPROPERTY()
	bool bLastOptionThreeEnabled = false;
    
	UPROPERTY()
	bool bLastOptionThreeToggleState = false;
	
	UPROPERTY()
	TSoftObjectPtr<UStaticMesh> AsyncAsset;
	
	UPROPERTY()
	TSoftObjectPtr<UStaticMesh> AsyncOptionOneMesh;
	
	UPROPERTY()
	TSoftObjectPtr<UStaticMesh> AsyncOptionTwoMesh;

	UPROPERTY()
	TSoftObjectPtr<UStaticMesh> AsyncOptionThreeMesh;

	UPROPERTY()
	TSoftObjectPtr<UStaticMesh> AsyncOptionThreeMesh2;

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
	void LoadAssetAsync(FName ProductName, int32 VariantIndex = 0, int32 VariantSizeIndex = 0, int32 MaterialIndex = 0, bool bOptionOne = false, bool bOptionTwo = false, bool bOptionThree = true);

	/**
	 * Internal Functions
	 */
	void OnAssetLoaded();
	void Initialize();
};
