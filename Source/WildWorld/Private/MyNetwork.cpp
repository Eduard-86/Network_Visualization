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

	//StartSimulator();
}

void AMyNetwork::StartSimulator()
{
	UWorld* World = GetWorld();

	SettiStruct.EpsilonLocation = 5;

	if (World)
	{
		AMyNodeActor* NewNode = Cast<AMyNodeActor>(World->SpawnActor(ComponentClass));


		ArrayNodeOnTheWorld.Add(NewNode);

		NewNode->SetActorLocation(FVector(
			1 + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1)),
			(10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1)),
			300 + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1))));

		for (int i = 1; i < start_size; ++i)
		{
			 NewNode = Cast<AMyNodeActor>(World->SpawnActor(ComponentClass));
			

			
			ArrayNodeOnTheWorld.Add(NewNode);

			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, 
				FString::FromInt(FMath::Rand() % SettiStruct.EpsilonLocation));

			NewNode->SetActorLocation(FVector(
				1 + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1)),
				(150 * i) + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1)),
				300 + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1))));


			
			DrawDebugLine(World, ArrayNodeOnTheWorld[i - 1]->GetActorLocation(),
				ArrayNodeOnTheWorld[i]->GetActorLocation(), 
				FColor::Red, false, 2, 2, 3.f);

			//NewNode->SubscribeOnNode(ArrayNodeOnTheWorld[i-1]);

			//NewNode->SubDel.AddDynamic(this, &AMyNodeActor::CallEvent);
		}
	}


	// ну сделай нормально заебал 
	for (int i = 0; i < ArrayNodeOnTheWorld.Num() - 1; ++i)
	{
		ArrayNodeOnTheWorld[i + 1]->SubscribeOnMe(ArrayNodeOnTheWorld[i], ESubType::Sum);

		ArrayNodeOnTheWorld[i]->MySubscription.Emplace(FSubData(ArrayNodeOnTheWorld[i + 1], ESubType::Sum));
		
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

	// Убийство одиноких
	for (int i = ArrayNodeOnTheWorld.Num() - 1; i > -1; --i)
	{
		if (ArrayNodeOnTheWorld[i]->IsEmpty())
		{
			FString Masage("Node - " + ArrayNodeOnTheWorld[i]->GetName()+ "it is delete ");
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, Masage);

			ArrayNodeOnTheWorld[i]->Destroy();
			
			ArrayNodeOnTheWorld.RemoveAt(i);
		}
	}

	

	for(int i = 0; i < ArrayNodeOnTheWorld.Num(); i++)
	{
		int32 RandIndex = FMath::Rand() % 100;

		if (RandIndex <= perCallEvet)
		{
			ArrayNodeOnTheWorld[i]->BroatcastEvectsAllSubs();
			
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, 
				FString("Node - " + ArrayNodeOnTheWorld[i]->GetName() + " CallEvent"));

		}
		else if (RandIndex <= perSubscribeOnNode)
		{
			ArrayNodeOnTheWorld[i]->SubscribeOnNode();

			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green,
				FString("Node - " + ArrayNodeOnTheWorld[i]->GetName() + " Subscribe"));
		}
		else if (RandIndex <= perUnSubscribe)
		{
			ArrayNodeOnTheWorld[i]->UnSubscribe();

			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red ,
				FString("Node - " + ArrayNodeOnTheWorld[i]->GetName() + " UnSubscribe"));
		}
		else if (RandIndex <= perCreatandSub)
		{
			// Перенести этот мусор в ноду

			//AMyNodeActor* NewNode = ArrayNodeOnTheWorld[TickCounter]->CreateAndSubscribeNewNode();
			AMyNodeActor* NewNode = Cast<AMyNodeActor>(World->SpawnActor(ComponentClass));

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

			////////////////////////////////

			ArrayNodeOnTheWorld.Add(NewNode);

			int32 CounterSub = ArrayNodeOnTheWorld[i]->MySubscription.Num();

			FVector PerentLocation = ArrayNodeOnTheWorld[i]->GetActorLocation();

			NewNode->SetActorLocation(FVector(
				PerentLocation.X + (150 + (FMath::Rand() % SettiStruct.EpsilonLocation)),
				PerentLocation.Y + (FMath::Rand() % SettiStruct.EpsilonLocation),
				(CounterSub * PerentLocation.Z + (FMath::Rand() % SettiStruct.EpsilonLocation))));


			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan,
				FString("Node - " + ArrayNodeOnTheWorld[i]->GetName() + " CreatandSub"));

		}
		else if (RandIndex <= perInaction)
		{
			ArrayNodeOnTheWorld[i]->Inaction();

			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Black,
				FString("Node - " + ArrayNodeOnTheWorld[i]->GetName() + " Inaction"));
		}

		/*
		 * Сюда нужна задержка 
		 */
	}

	// Отрисовка линий 
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
					false, 2, 2, 3.f);
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

	
	
	////////////////////////////////////////
	
	/*
	
	if(ArrayNodeOnTheWorld.Num() - 1 < TickCounter)
	{
		TickCounter = 0;
	}

	

	int32 RandIndex = FMath::Rand() % 100;

	if (RandIndex <= perCallEvet)
	{
		ArrayNodeOnTheWorld[TickCounter]->BroatcastEvectsAllSubs();
	}
	else if (RandIndex <= perSubscribeOnNode)
	{
		ArrayNodeOnTheWorld[TickCounter]->SubscribeOnNode();
	}
	else if (RandIndex <= perUnSubscribe)
	{
		ArrayNodeOnTheWorld[TickCounter]->UnSubscribe();
	}
	else if (RandIndex <= perCreatandSub)
	{
		// Перенести этот мусор в ноду

		UWorld* World = GetWorld();
		
		//AMyNodeActor* NewNode = ArrayNodeOnTheWorld[TickCounter]->CreateAndSubscribeNewNode();
		AMyNodeActor* NewNode = Cast<AMyNodeActor>(World->SpawnActor(ComponentClass));

		if (FMath::Rand() % 2 == 0)
		{
			NewNode->SubscribeOnMe(ArrayNodeOnTheWorld[TickCounter], ESubType::Counter);

			ArrayNodeOnTheWorld[TickCounter]->MySubscription.Emplace(FSubData(NewNode, ESubType::Counter));

		}
		else
		{
			NewNode->SubscribeOnMe(ArrayNodeOnTheWorld[TickCounter], ESubType::Sum);

			ArrayNodeOnTheWorld[TickCounter]->MySubscription.Emplace(FSubData(NewNode, ESubType::Sum));
		}

		////////////////////////////////

		ArrayNodeOnTheWorld.Add(NewNode);

		int32 CounterSub = ArrayNodeOnTheWorld[TickCounter]->MySubscription.Num();

		FVector PerentLocation = ArrayNodeOnTheWorld[TickCounter]->GetActorLocation();
		
		NewNode->SetActorLocation(FVector(
			PerentLocation.X + (150 + (FMath::Rand() % SettiStruct.EpsilonLocation)),
			PerentLocation.Y + (FMath::Rand() % SettiStruct.EpsilonLocation),
			(CounterSub * PerentLocation.Z +  (FMath::Rand() % SettiStruct.EpsilonLocation))));
		
		
		//NewNode->SetActorLocation(FVector(
		//	(150 * TickCounter) + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1)),
		//	1 + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1)),
		//	300 + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1))));
		
		
		int32 TempIndPerent = TickCounter;
		int32 TempIndChild = ArrayNodeOnTheWorld.Num() - 1;

		
		DrawDebugLine(World, ArrayNodeOnTheWorld[TempIndChild]->GetActorLocation(),
			ArrayNodeOnTheWorld[TempIndPerent]->GetActorLocation(), FColor::Red,
			false, 999999, 2, 3.f);
		
	}
	else if (RandIndex <= perInaction)
	{
		ArrayNodeOnTheWorld[TickCounter]->Inaction();
	}

	TickCounter++;

	return;
	*/

	//////////////////////

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