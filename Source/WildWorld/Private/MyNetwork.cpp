// Fill out your copyright notice in the Description page of Project Settings.


#include "MyNetwork.h"
#include "MyNodeActor.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

// Sets default values
AMyNetwork::AMyNetwork()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AMyNetwork::BeginPlay()
{
	Super::BeginPlay();

	//////////////////////////// Приберусь /////////////////////////////////
	///	AMyNodeActor* NewNode1 = Cast<AMyNodeActor>(World->SpawnActor(ComponentClass)); - получение мира
	///	
	/// NewNode1->SetActorLocation(FVector(1 + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1)), - изменение локации
	///		(10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1)),
	///		300 + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1))));
	///
	///	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green,
	///		FString::FromInt(FMath::Rand() % SettiStruct.EpsilonLocation));		-	дебаг масаге
	///
	/// DrawDebugLine(World, ArrayNodeOnTheWorld[i - 1]->GetActorLocation(),	-	 дебаг линия
	///		ArrayNodeOnTheWorld[i]->GetActorLocation(), FColor::Red, false, 999999, 2, 3.f);
	///
	///	GetWorldTimerManager().SetTimer(					- запустить таймер
	///		TimerHandle, this, & AMyNetwork::CreateEvent,
	///		SettiStruct.TimerRade, true);
	///
	///
	////////////////////////////////////////////////////////////////

	
}

void AMyNetwork::StartSimulator()
{
	GetWorldTimerManager().SetTimer(
		TimerHandle, this, &AMyNetwork::WorkTick,
		SettiStruct.TimerRade, true);


	UWorld* World = GetWorld();

	SettiStruct.EpsilonLocation = 5;

	if (World)
	{
		AMyNodeActor* NewNode1 = Cast<AMyNodeActor>(World->SpawnActor(ComponentClass));

		ArrayNodeOnTheWorld.Add(NewNode1);

		NewNode1->SetActorLocation(FVector(1 + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1)),
			(10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1)),
			300 + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1))));


		for (int i = 1; i < start_size; ++i)
		{
			AMyNodeActor* NewNode = Cast<AMyNodeActor>(World->SpawnActor(ComponentClass));

			ArrayNodeOnTheWorld.Add(NewNode);

			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, FString::FromInt(FMath::Rand() % SettiStruct.EpsilonLocation));

			NewNode->SetActorLocation(FVector(1 + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1)),
				(150 * i) + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1)),
				300 + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1))));

			DrawDebugLine(World, ArrayNodeOnTheWorld[i - 1]->GetActorLocation(),
				ArrayNodeOnTheWorld[i]->GetActorLocation(), FColor::Red, false, 999999, 2, 3.f);


			//NewNode->SubscribeOnNode(ArrayNodeOnTheWorld[i-1]);

			//NewNode->SubDel.AddDynamic(this, &AMyNodeActor::CallEvent);
		}
	}
}


void AMyNetwork::WorkTick()
{
	// Здесь должен быть рандомный эвент

	
	UWorld* World = GetWorld();
	
	AMyNodeActor* NewNode = Cast<AMyNodeActor>(World->SpawnActor(ComponentClass));

	ArrayNodeOnTheWorld.Add(NewNode);

	int y = ArrayNodeOnTheWorld.Num() - 1;

	
	NewNode->SetActorLocation(FVector(1 + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1)),
		(150 * y) + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1)),
		300 + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1))));

	DrawDebugLine(World, ArrayNodeOnTheWorld[y - 1]->GetActorLocation(),
		ArrayNodeOnTheWorld[y]->GetActorLocation(), FColor::Red, false, 999999, 2, 3.f);


	//NewNode->SubscribeOnNode(ArrayNodeOnTheWorld[i - 1]);

	/////////////////////////////////////////////////////////////
	/// Формула дебила - цифры настроек идут один за другим и мы 

	for (int i = 0; i < ArrayNodeOnTheWorld.Num(); ++i)
	{

		int32 RandIndex = FMath::Rand() % 100;

		if(RandIndex <= SettiStruct.callEvet)
		{
			ArrayNodeOnTheWorld[i]->BroatcastEvectsAllSubs();
		}
		else if(RandIndex > SettiStruct.callEvet && 
			RandIndex <= SettiStruct.callEvet + SettiStruct.subscribeOnNode)
		{
			
		}
		else if (RandIndex > SettiStruct.callEvet + SettiStruct.subscribeOnNode && 
			RandIndex <= SettiStruct.callEvet + SettiStruct.subscribeOnNode + SettiStruct.unSubscribe)
		{

		}
		else if (RandIndex > SettiStruct.callEvet + SettiStruct.subscribeOnNode + SettiStruct.unSubscribe &&
			RandIndex <= SettiStruct.callEvet + SettiStruct.subscribeOnNode + SettiStruct.unSubscribe + SettiStruct.creatandSub)
		{

		}
		else if (RandIndex > SettiStruct.callEvet + SettiStruct.subscribeOnNode +
			SettiStruct.unSubscribe + SettiStruct.creatandSub &&
				RandIndex <= SettiStruct.callEvet + SettiStruct.subscribeOnNode + SettiStruct.unSubscribe + 
					SettiStruct.creatandSub + SettiStruct.inaction)
		{

		}
	}
	
}

// Called every frame
void AMyNetwork::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	/*
	DrawDebugLine(World, ArrayNodeOnTheWorld[i - 1]->GetActorLocation(),
		ArrayNodeOnTheWorld[i]->GetActorLocation(), FColor::Red, false, 999999, 2, 3.f);
	*/
}

