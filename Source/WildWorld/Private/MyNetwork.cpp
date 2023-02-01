// Fill out your copyright notice in the Description page of Project Settings.


#include "MyNetwork.h"
#include "MyNodeActor.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(Network, All, All)

/*
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue,
		FString("Node - " + ArrayNodeOnTheWorld[i]->GetName() + " CallEvent"));
*/

// Sets default values
AMyNetwork::AMyNetwork()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void AMyNetwork::RemoveNetwork()
{
	for (AMyNodeActor* Node : ArrayNodeOnTheWorld)
		Node->Destroy();

	ArrayNodeOnTheWorld.Empty();

	GetWorldTimerManager().ClearTimer(TimerHandle);
}

// Called when the game starts or when spawned
void AMyNetwork::BeginPlay()
{
	Super::BeginPlay();
	
	SettiStruct.EpsilonLocation = 5;
}

void AMyNetwork::StartSimulator()
{
	UWorld* World = GetWorld();
	
	if (World)
	{	
		for(int i = 1; i <= start_size; ++i)
		{
			AMyNodeActor* NewNode = Cast<AMyNodeActor>(World->SpawnActor(NodeClass));

			ArrayNodeOnTheWorld.Add(NewNode);

			NewNode->SetActorLocation(FVector(
				1 + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1)),
				(150 * i) + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1)),
				300 + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1))));
			
		}

		// Balay 
		for (int i = 0; i < ArrayNodeOnTheWorld.Num(); ++i)
		{
			for (int j = 0; j < ArrayNodeOnTheWorld.Num(); ++j)
			{
				if(i != j)
				{
					if (FMath::Rand() % 2 == 0)
					{
						ArrayNodeOnTheWorld[j]->SubscribeOnMe(ArrayNodeOnTheWorld[i], ESubType::Counter);

						ArrayNodeOnTheWorld[i]->MySubscription.Emplace(FSubData(ArrayNodeOnTheWorld[j], ESubType::Counter));
					}
					else
					{
						ArrayNodeOnTheWorld[j]->SubscribeOnMe(ArrayNodeOnTheWorld[i], ESubType::Sum);

						ArrayNodeOnTheWorld[i]->MySubscription.Emplace(FSubData(ArrayNodeOnTheWorld[j], ESubType::Sum));
					}
				}
			}
		}

		perCallEvet = SettiStruct.callEvet;

		perSubscribeOnNode = SettiStruct.callEvet +
			SettiStruct.subscribeOnNode;

		perUnSubscribe = SettiStruct.callEvet +
			SettiStruct.subscribeOnNode +
			SettiStruct.unSubscribe;

		perCreatandSub = SettiStruct.callEvet +
			SettiStruct.subscribeOnNode +
			SettiStruct.unSubscribe +
			SettiStruct.creatandSub;

		perInaction = SettiStruct.callEvet +
			SettiStruct.subscribeOnNode +
			SettiStruct.unSubscribe +
			SettiStruct.creatandSub +
			SettiStruct.inaction;

		GetWorldTimerManager().SetTimer(
			TimerHandle, this, &AMyNetwork::WorkTick,
			SettiStruct.TimerRade, true);
	}
	else
	{
		check(World);
		return;
	}
}

/*
 *	Формула
 *
 *	Сперва получаем все шансы например
 *	call - 20
 *	sub - 20
 *	unsub - 20
 *	creat - 20
 *	ina - 20
 *
 *	После нам нужно каждое последуюшие число суммировать с предыдущем
 *	call - 20
 *	sub - 40
 *	unsub - 60
 *	creat - 80
 *	ina - 100
 *
 *	что даст нам возможность ралять число от 0 - 100 и поэтапро проверять
 *	попадает ли оно в диапозон
 *		ральнули 25
 *		сперва проверили <= оно первого числа
 *		если нет то идём и проверяем с следующим и тд
 *
 *	по сути снизу так и сделанно правда оформленно по уебански
 */
void AMyNetwork::WorkTick()
{
	UWorld* World = GetWorld();

	// Line print
	int ArraysCounter;

	for (int i = 0; i < ArrayNodeOnTheWorld.Num(); ++i)
	{
		ArraysCounter = ArrayNodeOnTheWorld[i]->MySubscription.Num() +
			ArrayNodeOnTheWorld[i]->SubDel.GetAllObjects().Num();

		for (int j = 0; j < ArraysCounter; ++j)
		{
			if (j < ArrayNodeOnTheWorld[i]->MySubscription.Num())
			{
				DrawDebugLine(World, ArrayNodeOnTheWorld[i]->GetActorLocation(),
					ArrayNodeOnTheWorld[i]->MySubscription[j].SubNode->GetActorLocation(),
					FColor::Red,
					false, SettiStruct.TimerRade + 0.2, 2, 3.f);
			}
			else
			{
				int TempCountDelArray = j - ArrayNodeOnTheWorld[i]->MySubscription.Num();

				AMyNodeActor* IsFind = Cast<AMyNodeActor>(ArrayNodeOnTheWorld[i]->SubDel.GetAllObjects()[TempCountDelArray]);

				DrawDebugLine(World, ArrayNodeOnTheWorld[i]->GetActorLocation(),
					IsFind->GetActorLocation(),
					FColor::Red,
					false,
					2, 2, 3.f);
			}
		}
	}

	// Убийство одиноких
	for (int i = ArrayNodeOnTheWorld.Num() - 1; i > -1; --i)
	{
		if (ArrayNodeOnTheWorld[i]->IsEmpty())
		{
			UE_LOG(Network, Warning, TEXT("Node - %s it is delete"), *ArrayNodeOnTheWorld[i]->GetName());
			
			ArrayNodeOnTheWorld[i]->Destroy();
			
			ArrayNodeOnTheWorld.RemoveAt(i);
		}
	}

	
	/**
	 * Events calling
	 */
	for(int i = 0; i < ArrayNodeOnTheWorld.Num(); i++)
	{
		int32 RandIndex = FMath::Rand() % 100;

		if (RandIndex <= perCallEvet)
		{
			ArrayNodeOnTheWorld[i]->BroatcastEvectsAllSubs();

			UE_LOG(Network, Display, TEXT("Node - %s CallEvent"), *ArrayNodeOnTheWorld[i]->GetName());
		}
		else if (RandIndex <= perSubscribeOnNode)
		{
			ArrayNodeOnTheWorld[i]->SubscribeOnNode();

			UE_LOG(Network, Display, TEXT("Node - %s Subscribe"), *ArrayNodeOnTheWorld[i]->GetName());
		}
		else if (RandIndex <= perUnSubscribe)
		{
			ArrayNodeOnTheWorld[i]->UnSubscribe();

			UE_LOG(Network, Display, TEXT("Node - %s UnSubscribe"), *ArrayNodeOnTheWorld[i]->GetName());
		}
		else if (RandIndex <= perCreatandSub)
		{
			// Перенести этот мусор в ноду, но пока оставим ибо атомарность 
			
			AMyNodeActor* NewNode = Cast<AMyNodeActor>(World->SpawnActor(NodeClass));

			if (FMath::Rand() % 2 == 0)
			{
				NewNode->SubscribeOnMe(ArrayNodeOnTheWorld[i], ESubType::Counter);

				ArrayNodeOnTheWorld[i]->MySubscription.Emplace(FSubData(NewNode, ESubType::Counter));

			}
			else
			{
				NewNode->SubscribeOnMe(ArrayNodeOnTheWorld[i], ESubType::Sum);

				ArrayNodeOnTheWorld[i]->MySubscription.Emplace(FSubData(NewNode, ESubType::Sum));
			}

			ArrayNodeOnTheWorld.Add(NewNode);
			
			////////////////////////////////

			FVector PerentLocation = ArrayNodeOnTheWorld[i]->GetActorLocation();

			int32 CornerAlpha = FMath::Rand() % 320;

			NewNode->SetActorLocation(FVector(
				PerentLocation.X +
				FMath::Cos(CornerAlpha) * ArrayNodeOnTheWorld[i]->WidthHeirLocation +
				(FMath::Rand() % SettiStruct.EpsilonLocation),

				PerentLocation.Y +
				FMath::Sin(CornerAlpha) * ArrayNodeOnTheWorld[i]->WidthHeirLocation +
				(FMath::Rand() % SettiStruct.EpsilonLocation),

				PerentLocation.Z + ArrayNodeOnTheWorld[i]->HeightHeirLocation +
				(FMath::Rand() % SettiStruct.EpsilonLocation)
			));

			/* old new node location
			 *
			 * int32 CounterSub = ArrayNodeOnTheWorld[i]->MySubscription.Num();
			 * 
			NewNode->SetActorLocation(FVector(
				PerentLocation.X + (150 + (FMath::Rand() % SettiStruct.EpsilonLocation)),
				PerentLocation.Y + (FMath::Rand() % SettiStruct.EpsilonLocation),
				(CounterSub * PerentLocation.Z + (FMath::Rand() % SettiStruct.EpsilonLocation))));
				*/

			UE_LOG(Network, Display, TEXT("Node - %s CreatandSub"), *ArrayNodeOnTheWorld[i]->GetName());
		}
		else if (RandIndex <= perInaction)
		{
			ArrayNodeOnTheWorld[i]->Inaction();

			UE_LOG(Network, Display, TEXT("Node - %s Inaction"), *ArrayNodeOnTheWorld[i]->GetName());
		}

		/*
		 * Сюда нужна задержка 
		 */
	}

	NetworkIterationDelegate.Broadcast();
	
	// Loop end flag 
	UE_LOG(Network, Display, TEXT(
		"=========================================== Raund end ==========================================="));

	////////////////////////////////////////

	/*
	UWorld* World = GetWorld();
	
	AMyNodeActor* NewNode = Cast<AMyNodeActor>(World->SpawnActor(AMyNodeActor::StaticClass()));

	ArrayNodeOnTheWorld.Add(NewNode);

	int eleemIndex = ArrayNodeOnTheWorld.Num() - 1;

	
	NewNode->SetActorLocation(FVector(1 + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1)),
		(150 * eleemIndex) + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1)),
		300 + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1))));

	DrawDebugLine(World, ArrayNodeOnTheWorld[eleemIndex - 1]->GetActorLocation(),
		ArrayNodeOnTheWorld[eleemIndex]->GetActorLocation(), FColor::Red, false, 999999, 2, 3.f);


	//NewNode->SubscribeOnNode(ArrayNodeOnTheWorld[i - 1]);

	for (int i = 0; i < ArrayNodeOnTheWorld.Num(); ++i)
	{

		int32 RandIndex = FMath::Rand() % 100;

		if (RandIndex <= perCallEvet)
		{
			//ArrayNodeOnTheWorld[i]->BroatcastEvectsAllSubs();
		}
		else if (RandIndex <= perSubscribeOnNode)
		{

		}
		else if (RandIndex <= perUnSubscribe)
		{

		}
		else if (RandIndex <= perCreatandSub)
		{

		}
		else if (RandIndex <= perInaction)
		{

		}
	}
	*/
}