// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StreamableManager.h"
#include "ProductLoader.generated.h"

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
	bool bHalfFencingOptionAvailable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bHalfFencingOptionAvailable", EditConditionHides))
	TSoftObjectPtr<UStaticMesh> HalfFencingAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bFullFencingOptionAvailable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bFullFencingOptionAvailable", EditConditionHides))
	TSoftObjectPtr<UStaticMesh> FullFencingAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bGlassOptionAvailable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bGlassOptionAvailable", EditConditionHides))
	TSoftObjectPtr<UStaticMesh> GlassAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector Dimensions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Price = 0.f;

	FAssetDetails()
	{
		AssetName = TEXT("Default");
		Thumbnail = nullptr;
		bHalfFencingOptionAvailable = false;
		bFullFencingOptionAvailable = false;
		bGlassOptionAvailable = false;
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

UCLASS()
class AProductLoader : public AStaticMeshActor
{
	GENERATED_BODY()
	AProductLoader();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(RowType="ConfigurationData"), Category = Configuration)
	UDataTable* ConfigurationData = nullptr;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = Configuration)
	FName DefaultProductName = TEXT("Default");

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
	UStaticMeshComponent* HalfFencingMeshComp = nullptr;

	UPROPERTY()
	bool bHalfFencingVisible = false;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	UStaticMeshComponent* FullFencingMeshComp = nullptr;

	UPROPERTY()
	bool bFullFencingVisible = false;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	UStaticMeshComponent* GlassMeshComp = nullptr;

	UPROPERTY()
	bool bGlassVisible = false;
	
	UPROPERTY()
	TSoftObjectPtr<UStaticMesh> AsyncAsset;
	
	UPROPERTY()
	TSoftObjectPtr<UStaticMesh> AsyncHalfFencingMesh;
	
	UPROPERTY()
	TSoftObjectPtr<UStaticMesh> AsyncFullFencingMesh;

	UPROPERTY()
	TSoftObjectPtr<UStaticMesh> AsyncGlassMesh;

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
	void LoadAssetAsync(FName ProductName, int32 VariantIndex = 0, int32 VariantSizeIndex = 0, int32 MaterialIndex = 0, bool bHalfFencing = false, bool bFullFencing = false, bool bGlassOption = false);

	/**
	 * Internal Functions
	 */
	void OnAssetLoaded();
	void Initialize();
};
