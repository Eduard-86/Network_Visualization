// Fill out your copyright notice in the Description page of Project Settings.


#include "ObserverPawn.h"
#include "Components/InputComponent.h"

// Sets default values
AObserverPawn::AObserverPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AObserverPawn::BeginPlay()
{
	Super::BeginPlay();

	///
	IgnoreType.AddIgnoredActor(this);
	
}

// Called every frame
void AObserverPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	GetWorld()->LineTraceSingleByObjectType(HitNode, this->GetActorLocation(), 
		this->GetActorLocation() + this->GetActorForwardVector() * 800, FCollisionObjectQueryParams::AllDynamicObjects, IgnoreType);

}

// Called to bind functionality to input
void AObserverPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AObserverPawn::MoveForward );
	PlayerInputComponent->BindAxis("MoveRight", this, &AObserverPawn::MoveRight );
	PlayerInputComponent->BindAxis("LookUp", this, &AObserverPawn::LookUp);
	PlayerInputComponent->BindAxis("Turn", this, &AObserverPawn::TurnAround);
	PlayerInputComponent->BindAxis("MoveUp", this, &AObserverPawn::MoveUp);

}

void AObserverPawn::MoveForward(float val)
{
	if(val)
	{
		// also for characters
		//AddMovementInput(GetActorForwardVector(), val);

		SetActorLocation(GetActorLocation() + (GetActorForwardVector() * (val * SpeedMultiplier)));
	}
}


void AObserverPawn::MoveRight(float val)
{
	if (val)
	{
		SetActorLocation(GetActorLocation() + (GetActorRightVector() * (val * SpeedMultiplier)));
	}
}

void AObserverPawn::LookUp(float val)
{
	if(val)
	{
		FRotator TempRot = GetActorRotation();
		TempRot.Pitch += ( -1 * val );
		SetActorRotation(TempRot);
	}
}

void AObserverPawn::TurnAround(float val)
{
	if(val)
	{
		FRotator TempRot = GetActorRotation();
		TempRot.Yaw += val;
		SetActorRotation(TempRot);
	}
}

void AObserverPawn::MoveUp(float val)
{
	if (val)
	{
		SetActorLocation(GetActorLocation() + (GetActorUpVector() * (val * SpeedMultiplier)));
	}
}
