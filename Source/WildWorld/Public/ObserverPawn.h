// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ObserverPawn.generated.h"

UCLASS()
class WILDWORLD_API AObserverPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AObserverPawn();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int SpeedMultiplier;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveForward(float val);
	void MoveRight(float val);

	void LookUp(float val);
	void TurnAround(float val);
	
	void MoveUp(float val);
		
};
