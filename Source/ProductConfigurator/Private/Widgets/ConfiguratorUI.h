// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ProductLoader.h"
#include "ConfiguratorUI.generated.h"

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

	UFUNCTION(BlueprintImplementableEvent, Category = "Configurator")
	void HandlePrimaryClick(FVector2D MousePos);

	UFUNCTION(BlueprintImplementableEvent, Category = "Configurator")
	void HandleSecondaryClick();

	UFUNCTION(BlueprintImplementableEvent, Category = "Configurator")
	void HandleToggle();

private:
	/**
	 *Internal Variables
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Configurator", meta = (AllowPrivateAccess = "true"));
	AProductLoader* ProductLoader;
	
};
