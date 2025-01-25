// Fill out your copyright notice in the Description page of Project Settings.


#include "SmoothCameraPawn.h"
#include "CineCameraComponent.h"
#include "ConfiguratorGameMode.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/ConfiguratorUI.h"


// Sets default values
ASmoothCameraPawn::ASmoothCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// Constructing Components
	PawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>("PawnMovement");
	
	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	SetRootComponent(Sphere);
	Sphere->SetCollisionResponseToAllChannels(ECR_Block);
	Sphere->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	Camera = CreateDefaultSubobject<UCineCameraComponent>("CineCamera");
	Camera->SetupAttachment(Sphere);
	Camera->bUsePawnControlRotation = true;
}

// Called when the game starts or when spawned
void ASmoothCameraPawn::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PlayerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MappingContext, 0);
		}
	}

	ProductLoader = Cast<AConfiguratorGameMode>(UGameplayStatics::GetGameMode(GetWorld()))->GetProductLoader();
}

void ASmoothCameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TraceForFocus();
}

void ASmoothCameraPawn::Move(const FInputActionValue& Value)
{
	const FVector LookVector = Value.Get<FVector>();

	if (Camera)
	{
		PawnMovement->AddInputVector(Camera->GetForwardVector() * LookVector.Y);
		PawnMovement->AddInputVector(Camera->GetRightVector() * LookVector.X);
		PawnMovement->AddInputVector(Camera->GetUpVector() * LookVector.Z);
	}
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
	if (ProductLoader)
	{
		ProductLoader->GetConfigUI()->HandleToggle();
	}
	else
	{
		ProductLoader = Cast<AConfiguratorGameMode>(UGameplayStatics::GetGameMode(GetWorld()))->GetProductLoader();
		ProductLoader->GetConfigUI()->HandleToggle();
	}
}

void ASmoothCameraPawn::HandlePrimaryClick(const FInputActionValue& Value)
{
	if (ProductLoader)
	{
		if (bIsMouseOver)
		{
			ProductLoader->GetConfigUI()->HandlePrimaryClick(UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld()));
		}
	}
	else
	{
		ProductLoader = Cast<AConfiguratorGameMode>(UGameplayStatics::GetGameMode(GetWorld()))->GetProductLoader();
		if (bIsMouseOver)
		{
			ProductLoader->GetConfigUI()->HandlePrimaryClick(UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld()));
		}
	}
}

void ASmoothCameraPawn::HandleSecondaryClick(const FInputActionValue& Value)
{
	if (ProductLoader)
	{
		ProductLoader->GetConfigUI()->HandleSecondaryClick();
	}
	else
	{
		ProductLoader = Cast<AConfiguratorGameMode>(UGameplayStatics::GetGameMode(GetWorld()))->GetProductLoader();
		ProductLoader->GetConfigUI()->HandleSecondaryClick();
	}
}

void ASmoothCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if(UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(IA_Fly, ETriggerEvent::Triggered, this, &ASmoothCameraPawn::Move);
		EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ASmoothCameraPawn::Look);
		EnhancedInputComponent->BindAction(IA_ToggleUI, ETriggerEvent::Started, this, &ASmoothCameraPawn::ToggleUI);
		EnhancedInputComponent->BindAction(IA_PrimaryClick, ETriggerEvent::Triggered, this, &ASmoothCameraPawn::HandlePrimaryClick);
		EnhancedInputComponent->BindAction(IA_SecondaryClick, ETriggerEvent::Triggered, this, &ASmoothCameraPawn::HandleSecondaryClick);
	}
}

void ASmoothCameraPawn::TraceForFocus()
{
	if (!PlayerController || !ProductLoader)
	{
		return;
	}

	// Get mouse position
	float MouseX, MouseY;
	if (!PlayerController->GetMousePosition(MouseX, MouseY))
	{
		if (bIsMouseOver)
		{
			bIsMouseOver = false;
			ProductLoader->OnMouseExitMesh();
		}
		return;
	}

	// Convert mouse position to world space
	FVector WorldLocation, WorldDirection;
	if (PlayerController->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection))
	{
		// Calculate end point
		FVector EndLocation = WorldLocation + (WorldDirection * 10000.0f);

		// Setup trace parameters
		FHitResult HitResult;
		TArray<AActor*> ActorsToIgnore;
		
		bool bHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(),
			WorldLocation,
			EndLocation,
			UEngineTypes::ConvertToTraceType(ECC_Visibility),
			false,
			ActorsToIgnore,
			EDrawDebugTrace::None,
			HitResult,
			false
			);

		// Check if we hit our static mesh component
		if (bHit && HitResult.GetActor() == ProductLoader)
		{
			if (!bIsMouseOver)
			{
				bIsMouseOver = true;
			}
			ProductLoader->OnMouseOverMesh();
		}
		else if (bIsMouseOver)
		{
			bIsMouseOver = false;
			ProductLoader->OnMouseExitMesh();
		}
	}
}


