// Fill out your copyright notice in the Description page of Project Settings.


#include "Orb.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Math.h"

AOrb::AOrb()
{
	PrimaryActorTick.bCanEverTick = false;

	OrbMesh = CreateDefaultSubobject<UStaticMeshComponent>("OrbMesh");
	SetRootComponent(OrbMesh);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 600.0f;
	CameraBoom->SetRelativeRotation(FRotator(-60.0f, 0.0f, 0.0f));
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void AOrb::BeginPlay()
{
	Super::BeginPlay();
}

void AOrb::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(MappingContext, 0);
		}
	}
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AOrb::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AOrb::Look);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AOrb::Move(const FInputActionValue& Value)
{
	FVector2D InputValue = Value.Get<FVector2D>();
	FVector MovementVector = InputValue.X * FollowCamera->GetForwardVector() * Speed + InputValue.Y * FollowCamera->GetRightVector() * Speed;
	OrbMesh->AddForce(FVector(MovementVector.X, MovementVector.Y, 0.0), FName(NAME_None),true);
	AOrb::LimitVelocity();
}

void AOrb::LimitVelocity()
{
	FVector TempVelocity(GetVelocity().X, GetVelocity().Y, 0.0);
	FVector ClampedVelocity = TempVelocity.GetClampedToSize(0.0, Speed);
	OrbMesh->SetPhysicsLinearVelocity(FVector(ClampedVelocity.X, ClampedVelocity.Y, GetVelocity().Z), false);
}

void AOrb::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	float NewPitch = FMath::Clamp<float>((CameraBoom->GetRelativeRotation().Pitch + LookAxisVector.Y), -60.0f, 60.0f);
	float NewYaw = CameraBoom->GetRelativeRotation().Yaw + LookAxisVector.X;
	CameraBoom->SetRelativeRotation(FRotator(NewPitch, NewYaw, 0.0f));
}

