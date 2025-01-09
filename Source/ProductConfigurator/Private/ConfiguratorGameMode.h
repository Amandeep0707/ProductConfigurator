// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ConfiguratorGameMode.generated.h"

class AProductLoader;
/**
 * 
 */
UCLASS()
class PRODUCTCONFIGURATOR_API AConfiguratorGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	FORCEINLINE AProductLoader* GetProductLoader() const { return ProductLoader; }
	FORCEINLINE void SetProductLoader(AProductLoader* InProductLoader) { this->ProductLoader = InProductLoader; }

private:
	UPROPERTY()
	AProductLoader* ProductLoader;
};
