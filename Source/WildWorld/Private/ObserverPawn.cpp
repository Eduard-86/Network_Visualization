// Fill out your copyright notice in the Description page of Project Settings.


#include "ObserverPawn.h"

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
	
}

// Called every frame
void AObserverPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AObserverPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

