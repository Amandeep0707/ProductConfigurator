// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ConfiguratorUI.generated.h"

class AProductLoader;
/**
 * 
 */
UCLASS()
class PRODUCTCONFIGURATOR_API UConfiguratorUI : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;

	/*
	 *	Inlines
	 */
	FORCEINLINE void SetLoader(AProductLoader* Loader) { this->ProductLoader = Loader; }

private:
	/**
	 *Internal Variables
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Configurator", meta = (AllowPrivateAccess = "true"));
	AProductLoader* ProductLoader;
	
};
