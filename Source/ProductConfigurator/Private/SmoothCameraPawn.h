// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Pawn.h"
#include "SmoothCameraPawn.generated.h"

class AProductLoader;
class UCineCameraComponent;
class USphereComponent;
class UFloatingPawnMovement;
class UInputMappingContext;
class UInputAction;

UCLASS()
class PRODUCTCONFIGURATOR_API ASmoothCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	ASmoothCameraPawn();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	/**
	 * Input Variables
	 */
	UPROPERTY(EditDefaultsOnly, Category = Inputs)
	UInputMappingContext* MappingContext;

	UPROPERTY(EditDefaultsOnly, Category = Inputs)
	UInputAction* IA_Fly;

	UPROPERTY(EditDefaultsOnly, Category = Inputs)
	UInputAction* IA_Look;

	UPROPERTY(EditDefaultsOnly, Category = Inputs)
	UInputAction* IA_ToggleUI;

	UPROPERTY(EditDefaultsOnly, Category = Inputs)
	UInputAction* IA_PrimaryClick;

	UPROPERTY(EditDefaultsOnly, Category = Inputs)
	UInputAction* IA_SecondaryClick;

	/**
	 * Internal Variables
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UFloatingPawnMovement* PawnMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	USphereComponent* Sphere;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UCineCameraComponent* Camera;

	UPROPERTY()
	APlayerController* PlayerController;
	
	UPROPERTY()
	AProductLoader* ProductLoader;

	UPROPERTY()
	bool bIsMouseOver = false;
	
	/**
	 * Input Functions
	 */
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void ToggleUI(const FInputActionValue& Value);
	void HandlePrimaryClick(const FInputActionValue& Value);
	void HandleSecondaryClick(const FInputActionValue& Value);
	void TraceForFocus();
};
