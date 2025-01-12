// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Pawn.h"
#include "SmoothCameraPawn.generated.h"

class UCameraComponent;
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
	UInputAction* IA_LeftClick;

	UPROPERTY(EditDefaultsOnly, Category = Inputs)
	UInputAction* IA_RightClick;

	/**
	 * Internal Variables
	 */
	UPROPERTY(EditDefaultsOnly, Category = Components)
	UFloatingPawnMovement* PawnMovement;

	UPROPERTY(EditDefaultsOnly, Category = Components)
	USphereComponent* Sphere;

	UPROPERTY(EditDefaultsOnly, Category = Components)
	UCameraComponent* Camera;
	
	/**
	 * Input Functions
	 */
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void ToggleUI(const FInputActionValue& Value);
	void HandleLeftClick(const FInputActionValue& Value);
	void HandleRightClick(const FInputActionValue& Value);
};
