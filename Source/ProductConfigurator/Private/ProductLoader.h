// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Components/Widget.h"
#include "ProductLoader.generated.h"

USTRUCT(BlueprintType)
struct FConfigurationDetails : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName AssetName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UTexture2D* Thumbnail;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UStaticMesh> Asset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector Dimensions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Price = 0.f;

	FConfigurationDetails()
	{
		AssetName = TEXT("Default");
		Thumbnail = nullptr;
		Dimensions = FVector::ZeroVector;
		Price = 0.f;
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

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(RowType="ConfigurationData"), Category = Configuration)
	UDataTable* ConfigurationData;

	UPROPERTY(EditDefaultsOnly, Category = Configuration)
	FName DefaultProductName = TEXT("Default");

	UPROPERTY(EditDefaultsOnly, Category = Configuration)
	UWidget* ConfigUI;

private:

	/**
	 * Internal Variables
	 */
	UPROPERTY()
	TSoftObjectPtr<UStaticMesh> AsyncAsset;

	UPROPERTY()
	APlayerController* PlayerController;
	
	/**
	 * Exposed Functions
	 */
	UFUNCTION(BlueprintCallable, Category = Configuration)
	bool LoadAssetAsync(FName ProductName, int32 ConfigID);

	/**
	 * Internal Functions
	 */
	void OnAssetLoaded();
	void Initialize();
};
