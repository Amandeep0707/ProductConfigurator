// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Pawn.h"
#include "SmoothCameraPawn.generated.h"

class USpringArmComponent;
class AProductLoader;
class UCineCameraComponent;
class USphereComponent;
class UFloatingPawnMovement;
class UInputMappingContext;
class UInputAction;

UENUM()
enum EControlType
{
	Flying UMETA(DisplayName = "Flying"),
	Orbit UMETA(DisplayName = "Orbit"),
};

UCLASS()
class PRODUCTCONFIGURATOR_API ASmoothCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	ASmoothCameraPawn();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void UpdateCameraLocation();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	/**
	 * Input Variables
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Controls | Inputs")
	UInputMappingContext* MappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Controls | Inputs")
	UInputAction* IA_Fly;

	UPROPERTY(EditDefaultsOnly, Category = "Controls | Inputs")
	UInputAction* IA_Look;

	UPROPERTY(EditDefaultsOnly, Category = "Controls | Inputs")
	UInputAction* IA_ToggleUI;

	UPROPERTY(EditDefaultsOnly, Category = "Controls | Inputs")
	UInputAction* IA_PrimaryClick;

	UPROPERTY(EditDefaultsOnly, Category = "Controls | Inputs")
	UInputAction* IA_SecondaryClick;

	UPROPERTY(EditDefaultsOnly, Category = "Controls | Inputs")
	UInputAction* IA_Scroll;

	UPROPERTY(EditDefaultsOnly, Category = "Controls | Inputs")
	TEnumAsByte<EControlType> ControlType = EControlType::Orbit;

	UPROPERTY(EditDefaultsOnly, Category = "Controls | Inputs")
	float OrbitCameraDistance = 2.f;

	UPROPERTY(EditDefaultsOnly, Category = "Controls | Inputs")
	float ScrollSpeed = 30.f;

	/**
	 * Internal Variables
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UFloatingPawnMovement* PawnMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	USphereComponent* Sphere;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UCineCameraComponent* Camera;

	UPROPERTY()
	APlayerController* PlayerController;
	
	UPROPERTY()
	AProductLoader* ProductLoader;

	UPROPERTY()
	bool bIsMouseOver = false;

	UPROPERTY()
	float FocusDistance = 0.f;

	UPROPERTY()
	float MinCameraDistance = 100.f;
	
	/**
	 * Input Functions
	 */
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void ToggleUI(const FInputActionValue& Value);
	void HandlePrimaryClick(const FInputActionValue& Value);
	void HandleSecondaryClick(const FInputActionValue& Value);
	void HandleScroll(const FInputActionValue& Value);
	void TraceForFocus();
};
