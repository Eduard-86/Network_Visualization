// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "MyNetwork.generated.h"

class UMyNode;
class AMyNodeActor;

USTRUCT(BlueprintType)
struct FRandomSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 callEvet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 subscribeOnNode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 unSubscribe;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 creatandSub;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 inaction;

	///////////////////////////////////////////

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EpsilonLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimerRade = 3.0f;
};

/*
	Удаление узлов без связей
	вызов рандомного евента
	сохранение созданных элементов в основной массив
 */

UCLASS()
class WILDWORLD_API AMyNetwork : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyNetwork();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	TArray<AMyNodeActor*> ArrayNodeOnTheWorld;

	

	TArray<AMyNodeActor*> ArrayNodes;
	
	TArray<AMyNodeActor*> ArrayNewNodes;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRandomSettings SettiStruct;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 start_size;

	FTimerHandle TimerHandle;

	void WorkTick();

	void StartSimulator();

	//TArray<UMyNode> NodeArray;

	/////////////////////////////////////////////////////////
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = BP) 
	TSubclassOf<AMyNodeActor> ComponentClass;  
	/////////////////////////////////////////////////////////

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
