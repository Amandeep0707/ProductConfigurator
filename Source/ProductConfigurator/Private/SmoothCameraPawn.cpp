// Fill out your copyright notice in the Description page of Project Settings.


#include "SmoothCameraPawn.h"
#include "ConfiguratorGameMode.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/ConfiguratorUI.h"


// Sets default values
ASmoothCameraPawn::ASmoothCameraPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	// Constructing Components
	PawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>("PawnMovement");
	
	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	Sphere->SetupAttachment(GetRootComponent());
	Sphere->SetCollisionResponseToAllChannels(ECR_Block);
	Sphere->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(Sphere);
	Camera->bUsePawnControlRotation = true;
}

// Called when the game starts or when spawned
void ASmoothCameraPawn::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MappingContext, 0);
		}
	}
}

void ASmoothCameraPawn::Move(const FInputActionValue& Value)
{
	const FVector LookVector = Value.Get<FVector>();

	PawnMovement->AddInputVector(Camera->GetForwardVector()*LookVector.Y);
	PawnMovement->AddInputVector(Camera->GetRightVector()*LookVector.X);
	PawnMovement->AddInputVector(Camera->GetUpVector()*LookVector.Z);
}

void ASmoothCameraPawn::Look(const FInputActionValue& Value)
{
	const FVector2D LookVector = Value.Get<FVector2D>();

	if (Controller)
	{
		AddControllerYawInput(LookVector.X);
		AddControllerPitchInput(LookVector.Y);
	}
}

void ASmoothCameraPawn::ToggleUI(const FInputActionValue& Value)
{
	if (const AProductLoader* ProductLoader = Cast<AConfiguratorGameMode>(UGameplayStatics::GetGameMode(GetWorld()))->GetProductLoader())
	{
		ProductLoader->GetConfigUI()->HandleToggle();
	}
}

void ASmoothCameraPawn::HandleLeftClick(const FInputActionValue& Value)
{
	if (const AProductLoader* ProductLoader = Cast<AConfiguratorGameMode>(UGameplayStatics::GetGameMode(GetWorld()))->GetProductLoader())
	{
		if (ProductLoader->bIsMouseOver)
		{
			ProductLoader->GetConfigUI()->HandleLeftClick(UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld()));
		}
	}
}

void ASmoothCameraPawn::HandleRightClick(const FInputActionValue& Value)
{
	if (const AProductLoader* ProductLoader = Cast<AConfiguratorGameMode>(UGameplayStatics::GetGameMode(GetWorld()))->GetProductLoader())
	{
		ProductLoader->GetConfigUI()->HandleRightClick();
	}
}

// Called to bind functionality to input
void ASmoothCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if(UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(IA_Fly, ETriggerEvent::Triggered, this, &ASmoothCameraPawn::Move);
		EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ASmoothCameraPawn::Look);
		EnhancedInputComponent->BindAction(IA_ToggleUI, ETriggerEvent::Started, this, &ASmoothCameraPawn::ToggleUI);
		EnhancedInputComponent->BindAction(IA_LeftClick, ETriggerEvent::Triggered, this, &ASmoothCameraPawn::HandleLeftClick);
		EnhancedInputComponent->BindAction(IA_RightClick, ETriggerEvent::Triggered, this, &ASmoothCameraPawn::HandleRightClick);
	}
}

