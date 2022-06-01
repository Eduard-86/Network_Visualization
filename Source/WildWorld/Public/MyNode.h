// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "MyNode.generated.h"

//DECLARE_DELEGATE()

class UMyNode;

//DECLARE_MULTICAST_DELEGATE_OneParam(FEventDelegate, AActor*);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEventDelegate, float, P1);

//ClassGroup=(Custom), meta=(BlueprintSpawnableComponent)
UCLASS()
class WILDWORLD_API UMyNode : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMyNode();

	UPROPERTY(BlueprintAssignable)
	FEventDelegate Pricol;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
