// Copyright Epic Games, Inc. All Rights Reserved.

#include "MagicTechArtCharacter.h"
#include "MagicTechArtProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AMagicTechArtCharacter

AMagicTechArtCharacter::AMagicTechArtCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.0f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a DrawMagicComponent
	DrawMagicComponent = CreateDefaultSubobject<UDrawMagicComponent>(TEXT("DrawMagic"));
	DrawMagicComponent->SetupAttachment(GetCapsuleComponent());
	
	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	//FP_Gun->bCastDynamicShadow = false;
	//FP_Gun->CastShadow = false;
	FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);
}

void AMagicTechArtCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();


	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	//FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true), TEXT("GripPoint"));
	WandLocation = FP_Gun->GetRelativeLocation();
	WandRotation = FP_Gun->GetRelativeRotation();
}

void AMagicTechArtCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (IsLocked)
	{
		DrawMagicComponent->UpdateDrawing();
		
		FVector StartLocation = FP_Gun->GetComponentLocation();
		FVector Direction = DrawingTarget - StartLocation;
		Direction.Normalize();
		
		FRotator NewRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
		FP_Gun->SetWorldRotation(NewRotation);
		FP_Gun->AddRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
		
		/*
		APlayerController* PC = GetWorld()->GetFirstPlayerController();

		float LocationX, LocationY;
		PC->GetMousePosition(LocationX, LocationY);

		int32 ViewportWidth = 0;
		int32 ViewportHeight = 0;
		PC->GetViewportSize(ViewportWidth, ViewportHeight);

		int32 CenterX = ViewportWidth / 2;
		int32 CenterY = ViewportHeight / 2;
		FVector Offset(-(LocationX - CenterX) * 0.1f, 0, -(LocationY - CenterY) * 0.1f);

		FP_Gun->SetRelativeLocation(WandLocation + Offset + WandOffset);
		*/
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMagicTechArtCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMagicTechArtCharacter::StartJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AMagicTechArtCharacter::StopJump);

	// Bind fire event
	PlayerInputComponent->BindAction("Tab", IE_Pressed, this, &AMagicTechArtCharacter::OnTabPressed);
	PlayerInputComponent->BindAction("Tab", IE_Released, this, &AMagicTechArtCharacter::OnTabReleased);
	
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMagicTechArtCharacter::OnFirePressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AMagicTechArtCharacter::OnFireReleased);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AMagicTechArtCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMagicTechArtCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &AMagicTechArtCharacter::AddYaw);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMagicTechArtCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &AMagicTechArtCharacter::AddPitch);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMagicTechArtCharacter::LookUpAtRate);
}

/*
void AMagicTechArtCharacter::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != nullptr)
	{
		UWorld* const World = GetWorld();
		if (World != nullptr)
		{
			const FRotator SpawnRotation = GetControlRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

			// spawn the projectile at the muzzle
			World->SpawnActor<AMagicTechArtProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		}
	}

	// try and play the sound if specified
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}*/

void AMagicTechArtCharacter::OnTabPressed()
{
	if (IsLocked)
		return;
	
	if (GetCharacterMovement()->IsFalling())
		return;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	//PC->bShowMouseCursor = true;
	//PC->bEnableClickEvents = true;
	//PC->SetInputMode(FInputModeGameAndUI());

	int32 ViewportWidth = 0;
	int32 ViewportHeight = 0;
	PC->GetViewportSize(ViewportWidth, ViewportHeight);

	int32 CenterX = ViewportWidth / 2;
	int32 CenterY = ViewportHeight / 2;

	PC->SetMouseLocation(CenterX, CenterY);
	
	DrawMagicComponent->StartDrawing();
	GetCharacterMovement()->MaxWalkSpeed = 50.0f;
	
	IsLocked = true;
}

void AMagicTechArtCharacter::OnTabReleased()
{
	if (!IsLocked)
		return;

	//APlayerController* PC = GetWorld()->GetFirstPlayerController();
	//PC->bShowMouseCursor = false;
	//PC->bEnableClickEvents = true;
	//PC->SetInputMode(FInputModeGameOnly());

	IsLocked = false;

	OnFireReleased();
	
	FP_Gun->SetRelativeLocation(WandLocation);
	FP_Gun->SetRelativeRotation(WandRotation);

	DrawMagicComponent->StopDrawing();
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	IsDrawing = false;
}

void AMagicTechArtCharacter::OnFirePressed()
{
	if (!IsLocked && !IsDrawing)
		return;

	IsDrawing = true;
}

void AMagicTechArtCharacter::OnFireReleased()
{
	if (!IsDrawing)
		return;

	DrawMagicComponent->StopDrawing();
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	IsDrawing = false;

	OnTabReleased();
}

void AMagicTechArtCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AMagicTechArtCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AMagicTechArtCharacter::TurnAtRate(float Rate)
{
	if (IsLocked || IsDrawing)
		return;
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMagicTechArtCharacter::LookUpAtRate(float Rate)
{
	if (IsLocked || IsDrawing)
		return;
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMagicTechArtCharacter::AddPitch(float Val)
{
	if (IsLocked || IsDrawing)
		return;

	AddControllerPitchInput(Val);
}

void AMagicTechArtCharacter::AddYaw(float Val)
{
	if (IsLocked || IsDrawing)
		return;
	
	AddControllerYawInput(Val);
}

void AMagicTechArtCharacter::StartJump()
{
	if (IsLocked || IsDrawing)
		return;
	
	Jump();
}

void AMagicTechArtCharacter::StopJump()
{
	StopJumping();
}
