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
	int32 callEvet = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 subscribeOnNode = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 unSubscribe = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 creatandSub = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 inaction = 100;

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

	int32 perCallEvet = 0;
	int32 perSubscribeOnNode = 0;
	int32 perUnSubscribe = 0;
	int32 perCreatandSub = 0;
	int32 perInaction = 0;

	int32 TickCounter = 0;

	//int eventPercentArray[5];
	
public:	
	// Sets default values for this actor's properties
	AMyNetwork();

	UFUNCTION(BlueprintCallable)
	void StartSimulator();

	UFUNCTION(BlueprintCallable)
	int32 GetTickCounter()
	{
		return TickCounter;
	};

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void WorkTick();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRandomSettings SettiStruct;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 start_size;
	
	TArray<AMyNodeActor*> ArrayNodeOnTheWorld;

	//TArray<AMyNodeActor*> ArrayNodes;
	
	//TArray<AMyNodeActor*> ArrayNewNodesNotAlone;

	FTimerHandle TimerHandle;

	
	
	/////////////////////////////////////////////////////////
	//AMyNodeActor* NewNode1 = Cast<AMyNodeActor>(World->SpawnActor(ComponentClass));
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = BP) 
	TSubclassOf<AMyNodeActor> ComponentClass;  
	
	/////////////////////////////////////////////////////////
};
