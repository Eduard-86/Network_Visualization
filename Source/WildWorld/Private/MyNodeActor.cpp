// Fill out your copyright notice in the Description page of Project Settings.


#include "MyNodeActor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "MyNetwork.h"
#include "Materials/MaterialInstanceDynamic.h"



// Sets default values
AMyNodeActor::AMyNodeActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	Mesh->SetupAttachment(RootComponent);

	//Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	//Trigger->SetupAttachment(Mesh);
	
	HeightHeirLocation = 100 + FMath::Rand() % 200;
	WidthHeirLocation = 100 + FMath::Rand() % 200;

}

// Called when the game starts or when spawned
void AMyNodeActor::BeginPlay()
{
	Super::BeginPlay();

	//Trigger->OnComponentBeginOverlap.AddDynamic(this, &AMyNodeActor::BrotcastEvents);

	Mesh->OnComponentBeginOverlap.AddDynamic(this, &AMyNodeActor::OverlapBrotcastEvents);

	DynMater = Mesh->CreateAndSetMaterialInstanceDynamic(0);

	if(DynMater)
	{
		DynMater->SetVectorParameterValue("ColorVal", FLinearColor::White);
	}

	bool test = Mesh->OverlapComponent(GetActorLocation(), FQuat(), FCollisionShape());

	if(test)
	{
		int i = 0;
		i -= 20;
	}
	
}

// event for the collision,
// todo delete this later
void AMyNodeActor::OverlapBrotcastEvents(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//SubDel.Broadcast(FMath::Rand() % 100, this);

	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, *GetName());

}

void AMyNodeActor::SubscribeOnMe(AMyNodeActor* NewSubNode, ESubType SubType)
{
	check(NewSubNode);

	 int32 TempIndex = SubDel.GetAllObjects().Find(NewSubNode);

	if(TempIndex == -1)
	{
		switch (SubType)
		{
			case ESubType::Sum:
			{
				SubDel.AddDynamic(NewSubNode, &AMyNodeActor::SumEvent);
				//MySubscription.Emplace(FSubData(NewSubNode, ESubType::Sum, 0));
				break;
			}

			case ESubType::Counter:
			{
				SubDel.AddDynamic(NewSubNode, &AMyNodeActor::CounterEvent);
				//MySubscription.Emplace(FSubData(NewSubNode, ESubType::Counter, 0));
				break;
			}

			default:
			{
				check(false);
				break;
			}
		}
	}
	else
	{
		check(false);
		return;
	}

	
}

void AMyNodeActor::UnSubscribeOnMe(AMyNodeActor* NewSubNode, ESubType SubType)
{
	check(NewSubNode);

	UObject* IsFindObj = *SubDel.GetAllObjects().FindByPredicate(
		[&](UObject* SubData)
		{
			AMyNodeActor* IsFind = Cast<AMyNodeActor>(SubData);

			if (IsFind)
			{
				return IsFind == NewSubNode;
			}
			else
			{
				check(IsFind);
				return false;
			}
		});

	if(IsFindObj)
	{
		switch (SubType)
		{
			case ESubType::Sum:
			{
				SubDel.RemoveDynamic(NewSubNode, &AMyNodeActor::SumEvent);
				break;
			}
			case ESubType::Counter:
			{
				SubDel.RemoveDynamic(NewSubNode, &AMyNodeActor::CounterEvent);
				break;
			}

			default:
			{
				//TODO make something error report 
				check(false);
				break;
			}
		}
	}
	else
	{
		check(false);
		return;
	}
	
}

// evern for subscribe
void AMyNodeActor::SumEvent(float val, AMyNodeActor* who)
{
	FSubData* IsFind = MySubscription.FindByPredicate(
		[&](FSubData SubData)
		{
			return SubData.SubNode == who;
		});
	
	if(IsFind)
	{
		IsFind->SubValue = IsFind->SubValue + val;

		SumEventValue += IsFind->SubValue;
	}
	else
	{
		// ������ ���� ����� ����������� �� ������ �� �� ��������� ��� � ������ �������� 'MySubscription'
		check(false);
		return;
	}
	

#pragma region VisyalPrikols

	FString GetValueStr = FString::SanitizeFloat(IsFind->SubValue - val);

	FString AllSumStr = FString::SanitizeFloat(IsFind->SubValue);

	FString Masage("Event type - 'Sum'\nI cach ! - " + GetValueStr + " from "
		+ who->GetName() + "/nMy all back about this Node - " + AllSumStr);

	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, Masage);

	// set random color on the "colorval" property
	DynMater->SetVectorParameterValue("ColorVal", FLinearColor::MakeRandomColor());
	
#pragma endregion 
}

// evern for subscribe
void AMyNodeActor::CounterEvent(float val, AMyNodeActor* who)
{
	FSubData* IsFind = MySubscription.FindByPredicate(
		[&](FSubData SubData)
		{
			return SubData.SubNode == who;
		});

	if (IsFind)
	{
		IsFind->SubValue++;
		
		CounterEventValue++;
	}
	else
	{
		//������ ���� ����� ����������� �� ������ �� �� ��������� ��� � ������ �������� 'MySubscription'
		check(false);
		return;
	}

#pragma region VisyalPrikols

	FString AllSumStr = FString::SanitizeFloat(IsFind->SubValue);

	FString Masage("Event type - 'Counter' event catch from"+ who->GetName() 
		+ "/nMy all back about this Node - " + AllSumStr);

	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, Masage);

	// set random color on the "colorval" property
	DynMater->SetVectorParameterValue("ColorVal", FLinearColor::MakeRandomColor());

#pragma endregion 
}


#pragma region Events

void AMyNodeActor::BroatcastEvectsAllSubs()
{
	SubDel.Broadcast(FMath::Rand() % 100, this);
}

void AMyNodeActor::SubscribeOnNode()
{
	int32 my = MySubscription.Num();
	int32 del = SubDel.GetAllObjects().Num();

	if (!(my + del))
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, 
			TEXT("���� ���� �� ������� � �� �������� �����������"));
		return;
	}

	int32 indexsusa = FMath::Rand() % (my + del);

	AMyNodeActor* newsub = nullptr;
	
	if (indexsusa < my && my != 0)
	{
		int32 indexmysub = indexsusa == my ? indexsusa - 1 : indexsusa;

		newsub = MySubscription[indexmysub].SubNode;

		//int32 mysub_nei = newsub->ISubscription.Num();
		int32 mysub_nei = newsub->SubDel.GetAllObjects().Num();
		int32 del_nei = newsub->MySubscription.Num();
		int32 indexsusa_nei = FMath::Rand() % (mysub_nei + del_nei);

		if (indexsusa_nei < mysub_nei && mysub_nei != 0)
		{
			int32 indexmysub_nei = indexsusa_nei == mysub_nei ? indexsusa_nei - 1 : indexsusa_nei;

			newsub = Cast<AMyNodeActor, UObject>(newsub->SubDel.GetAllObjects()[indexmysub_nei]);
			
			//newsub = newsub->ISubscription[indexmysub_nei];

			check(newsub);
		}
		else if (del_nei != 0)
		{
			int32 inxdel = indexsusa_nei - mysub_nei;

			newsub = newsub->MySubscription[inxdel].SubNode;

			//newsub = newsub->MySubscription[inxdel];
			
			check(newsub);
		}

	}
	else if (del != 0)
	{
		int32 inxdel = indexsusa - my;

		newsub = Cast<AMyNodeActor, UObject>(SubDel.GetAllObjects()[inxdel]);

		//int32 mysub_nei = newsub->ISubscription.Num();
		int32 mysub_nei = newsub->SubDel.GetAllObjects().Num();

		int32 del_nei = newsub->MySubscription.Num();
		
		int32 indexsusa_nei = FMath::Rand() % (mysub_nei + del_nei);

		if (indexsusa_nei < mysub_nei && mysub_nei != 0)
		{
			int32 indexmysub_nei = indexsusa_nei == mysub_nei ? indexsusa_nei - 1 : indexsusa_nei;

			//newsub = newsub->ISubscription[indexmysub_nei];

			newsub = Cast<AMyNodeActor, UObject>(newsub->SubDel.GetAllObjects()[indexmysub_nei]);

			check(newsub);
		}
		else if (del_nei != 0)
		{
			inxdel = indexsusa_nei - mysub_nei;

			//newsub = newsub->MySubscription[inxdel];

			newsub = newsub->MySubscription[inxdel].SubNode;
			
			check(newsub);
		}

	}
	else
	{
		// false in array size 
		check(false);
	}

	if(newsub != this)
	{
		// �heck on sub, subscription this node on newnode earlier ?
		if (
			FSubData* IsFind = MySubscription.FindByPredicate(
				[&](FSubData SubData)
				{
					return SubData.SubNode == newsub;
				})
			)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red,
				TEXT("I subscripted on this node!"));
			return;
		}

		// Subscription successful !!!
		if (FMath::Rand() % 2 == 0)
		{
			newsub->SubscribeOnMe(this, ESubType::Counter);

			MySubscription.Emplace(FSubData(newsub, ESubType::Counter));

		}
		else
		{
			newsub->SubscribeOnMe(this, ESubType::Sum);

			MySubscription.Emplace(FSubData(newsub, ESubType::Sum));
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red,
			TEXT("Trying  subscription on his self!"));
		return;
	}
	
}

void AMyNodeActor::UnSubscribe()
{
	int32 size = MySubscription.Num();
	
	if(size)
	{
		int32 RandNodeIndex = FMath::Rand() % size;

		FSubData TempSub = MySubscription[RandNodeIndex];
			
		TempSub.SubNode->UnSubscribeOnMe(this, MySubscription[RandNodeIndex].SubType);

		MySubscription.Remove(TempSub);

	}
	else
	{
		// ���� �� ��� �� �� ���� ��������
		//check(size);
	}
}

AMyNodeActor* AMyNodeActor::CreateAndSubscribeNewNode()
{
	//todo
	// �� ������ ������ ������ ����� ������ ��������� ��������
	// ����� ������������ 

	/*
	UWorld* World = GetWorld();

	if (World)
	{
		AMyNodeActor* NewNode = Cast<AMyNodeActor>(World->SpawnActor(AMyNodeActor::StaticClass()));

		NewNode->SetActorTransform(this->GetActorTransform());

		//todo
		//NewNode->Mesh->OverlapComponent(NewNode->GetActorLocation(), FQuat(), FCollisionShape());

		return NewNode;
	}

	check(World);
	*/
	
	return nullptr;
}

