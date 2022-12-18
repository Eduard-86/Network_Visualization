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

	//////////////////////////// ��������� /////////////////////////////////
	///	AMyNodeActor* NewNode1 = Cast<AMyNodeActor>(World->SpawnActor(ComponentClass)); - ��������� ����
	///	
	/// NewNode1->SetActorLocation(FVector(1 + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1)), - ��������� �������
	///		(10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1)),
	///		300 + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1))));
	///
	///	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green,
	///		FString::FromInt(FMath::Rand() % SettiStruct.EpsilonLocation));		-	����� ������
	///
	/// DrawDebugLine(World, ArrayNodeOnTheWorld[i - 1]->GetActorLocation(),	-	 ����� �����
	///		ArrayNodeOnTheWorld[i]->GetActorLocation(), FColor::Red, false, 999999, 2, 3.f);
	///
	///	GetWorldTimerManager().SetTimer(					- ��������� ������
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
				FColor::Red, false, 999999, 2, 3.f);

			//NewNode->SubscribeOnNode(ArrayNodeOnTheWorld[i-1]);

			//NewNode->SubDel.AddDynamic(this, &AMyNodeActor::CallEvent);
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


void AMyNetwork::WorkTick()
{
	if(ArrayNodeOnTheWorld.Num() - 1 < TickCounter)
	{
		TickCounter = 0;
	}

	/*
	 *	�������
	 *
	 *	������ �������� ��� ����� ��������
	 *	call - 20
	 *	sub - 20
	 *	unsub - 20
	 *	creat - 20
	 *	ina - 20
	 *
	 *	����� ��� ����� ������ ����������� ����� ����������� � ����������
	 *	call - 20
	 *	sub - 40
	 *	unsub - 60
	 *	creat - 80
	 *	ina - 100
	 *
	 *	��� ���� ��� ����������� ������ ����� �� 0 - 100 � �������� ���������
	 *	�������� �� ��� � ��������
	 *		�������� 25
	 *		������ ��������� <= ��� ������� �����
	 *		���� ��� �� ��� � ��������� � ��������� � ��
	 *
	 *	�� ���� ����� ��� � �������� ������ ���������� �� ��������
	 */

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
		// ��������� ���� ����� � ����

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
		
		/*
		NewNode->SetActorLocation(FVector(
			(150 * TickCounter) + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1)),
			1 + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1)),
			300 + (10 * ((FMath::Rand() % SettiStruct.EpsilonLocation) + 1))));
		*/
		
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