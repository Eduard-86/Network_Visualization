// Fill out your copyright notice in the Description page of Project Settings.


#include "MyNodeActor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "MyNetwork.h"
#include "Materials/MaterialInstanceDynamic.h"
#include <utility>
#include <iterator>


// Sets default values
AMyNodeActor::AMyNodeActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	Mesh->SetupAttachment(RootComponent);

	//Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	//Trigger->SetupAttachment(Mesh);
	
}

// Called when the game starts or when spawned
void AMyNodeActor::BeginPlay()
{
	Super::BeginPlay();

	/* Подписка на все узлы на катре

		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMyNodeActor::StaticClass(), ArrayNodeOnTheWorld );

		for(int i = 0; i < ArrayNodeOnTheWorld.Num(); ++i)
		{
			AMyNodeActor* TempPtrNode = Cast<AMyNodeActor>(ArrayNodeOnTheWorld[i]);

			if(TempPtrNode)
			{
				if(TempPtrNode != this)
					TempPtrNode->SubDel.AddDynamic(this, &AMyNodeActor::CallEvent);
			}
		}

		Trigger->OnComponentBeginOverlap.AddDynamic(this, &AMyNodeActor::BrotcastEvents);
	 */

	//Trigger->OnComponentBeginOverlap.AddDynamic(this, &AMyNodeActor::BrotcastEvents);

	Mesh->OnComponentBeginOverlap.AddDynamic(this, &AMyNodeActor::BrotcastEvents);

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
void AMyNodeActor::BrotcastEvents(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	SubDel.Broadcast(FMath::Rand() % 100, this);

	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, *GetName());

}


void AMyNodeActor::SubscribeOnMe(AMyNodeActor* NewSubNode)
{
	SubDel.AddDynamic(NewSubNode, &AMyNodeActor::SumEvent);

	MySubscription.Emplace(FSubData(NewSubNode, 0, 0));
	

	//MySubscription.Add(NewSubNode);
}

void AMyNodeActor::UnSubscribeOnMe(AMyNodeActor* NewSubNode)
{
	check(NewSubNode);

	
	FSubData* IsFind = MySubscription.FindByPredicate(
	[&](FSubData SubData)
	{
		return SubData.SubNode == NewSubNode;
	});
	
	if(IsFind)
	{
		SubDel.RemoveDynamic(NewSubNode, &AMyNodeActor::SumEvent);
	}
	else
	{
		//todo make something error report 
	}
	
}

// evern for subscribe
void AMyNodeActor::SumEvent(float val, AMyNodeActor* who)
{
	FString StringFloat = FString::SanitizeFloat(val);
	
	FString Masage("I cach ! - " + StringFloat + " " + who->GetName());
	
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, Masage);

	// set random color on the "colorval" property
	DynMater->SetVectorParameterValue("ColorVal", FLinearColor::MakeRandomColor());
	
	/*
	if(val < 33)
	{
		DynMater->SetVectorParameterValue("ColorVal", FLinearColor::Red);
	}
	else if(val > 33 && val < 66)
	{
		DynMater->SetVectorParameterValue("ColorVal", FLinearColor::Green);
	}
	else
	{
		DynMater->SetVectorParameterValue("ColorVal", FLinearColor::Blue);
	}
	*/
	
	//Mesh->SetRelativeScale3D(Mesh->GetRelativeScale3D() + 0.1);

}

// evern for subscribe
void AMyNodeActor::CounterEvent(float val, AMyNodeActor* who)
{
	FString StringFloat = FString::SanitizeFloat(val);

	FString Masage("I cach ! - " + StringFloat + " " + who->GetName());

	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, Masage);

	// set random color on the "colorval" property
	DynMater->SetVectorParameterValue("ColorVal", FLinearColor::MakeRandomColor());
}


#pragma region Events

void AMyNodeActor::BroatcastEvectsAllSubs()
{
	SubDel.Broadcast(FMath::Rand() % 100, this);
}


void AMyNodeActor::EventSubscribeOnNode()
{
	//int32 my = ISubscription.Num();

	int32 my = SubDel.GetAllObjects().Num();
	int32 del = MySubscription.Num();

	

	if (my && del)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, 
			TEXT("Ноде пора на помойку а не подписки подписывать"));
		return;
	}

	int32 indexsusa = FMath::Rand() % (my + del);

	AMyNodeActor* newsub = nullptr;
	
	if (indexsusa < my && my != 0)
	{

		int32 indexmysub = indexsusa == my ? indexsusa - 1 : indexsusa;

		newsub = MySubscription[indexmysub].SubNode;

		//newsub = Cast<AMyNodeActor, UObject>(SubDel.GetAllObjects()[indexmysub]);

		//Cast<AMyNodeActor*, UObject*>(SubDel.GetAllObjects()[indexmysub]);
		
		
		//int32 mysub_nei = newsub->ISubscription.Num();
		int32 mysub_nei = newsub->SubDel.GetAllObjects().Num();
		int32 del_nei = newsub->MySubscription.Num();
		int32 indexsusa_nei = FMath::Rand() % (mysub_nei + del_nei);

		if (indexsusa_nei < mysub_nei && mysub_nei != 0)
		{
			int32 indexmysub_nei = indexsusa_nei == mysub_nei ? indexsusa_nei - 1 : indexsusa_nei;

			newsub = newsub->MySubscription[indexmysub_nei].SubNode;
			
			//newsub = newsub->ISubscription[indexmysub_nei];

			
			check(newsub);
		}
		else if (del_nei != 0)
		{
			int32 inxdel = indexsusa_nei - mysub_nei;

			newsub = Cast<AMyNodeActor, UObject>(newsub->SubDel.GetAllObjects()[inxdel]);

			//newsub = newsub->MySubscription[inxdel];
			
			check(newsub);
		}

	}
	else if (del != 0)
	{
		int32 inxdel = indexsusa - my;

		newsub = Cast<AMyNodeActor, UObject>(newsub->SubDel.GetAllObjects()[inxdel]);

		//newsub = MySubscription[inxdel];
		

		//int32 mysub_nei = newsub->ISubscription.Num();
		int32 mysub_nei = newsub->SubDel.GetAllObjects().Num();

		int32 del_nei = newsub->MySubscription.Num();
		
		int32 indexsusa_nei = FMath::Rand() % (mysub_nei + del_nei);

		if (indexsusa_nei < mysub_nei && mysub_nei != 0)
		{
			int32 indexmysub_nei = indexsusa_nei == mysub_nei ? indexsusa_nei - 1 : indexsusa_nei;

			//newsub = newsub->ISubscription[indexmysub_nei];

			newsub = newsub->MySubscription[indexmysub_nei].SubNode;

			check(newsub);
		}
		else if (del_nei != 0)
		{
			inxdel = indexsusa_nei - mysub_nei;

			//newsub = newsub->MySubscription[inxdel];

			newsub = Cast<AMyNodeActor, UObject>(newsub->SubDel.GetAllObjects()[inxdel]);

			check(newsub);
		}

	}
	else
	{
		// ошибка а размере массивов
		check(nullptr);
	}

	
	// Проверка на подписку, неподписан ли уже 
	if(
		FSubData* IsFind = MySubscription.FindByPredicate(
			[&](FSubData SubData)
			{
				return SubData.SubNode == newsub;
			})
		)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Узел был в подписках!"));
		return;
	}

	/*
	// Проверка на подписку, неподписан ли уже 
	if (MySubscription.Find(newsub))
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Узел был в подписках!"));
		return;
	}*/

	//if (ISubscription.Find(newsub))
	if(SubDel.GetAllObjects().Find(newsub))
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Мы были подписаны на узел!"));
		return;
	}

	// Ура подписка удолась !!!

	if(FMath::Rand() % 2 == 0)
	{
		newsub->SubDel.AddDynamic(this, &AMyNodeActor::SumEvent);

		MySubscription.Emplace(FSubData(newsub, true));
	}
	else
	{
		newsub->SubDel.AddDynamic(this, &AMyNodeActor::CounterEvent);

		MySubscription.Emplace(FSubData(newsub, false));
	}
	

	//ISubscription.Add(newsub);

	//newsub->SubscribeOnMe(this);

}


void AMyNodeActor::UnSubscribe(AMyNodeActor* NewSubNode)
{
	
	int32 size = SubDel.GetAllObjects().Num();
	
	if(size)
	{
		int32 randind = FMath::Rand() % size;

		//ISubscription[randind]->SubDel.RemoveDynamic(this, &AMyNodeActor::CallEvent);

		Cast<AMyNodeActor, UObject>(SubDel.GetAllObjects()[randind])->UnSubscribe(this);
		
		//NewSubNode->UnSubscribe(this);

		//ISubscription.RemoveAt(randind);
		
	}
	else
	{
		check(size);
	}
}

AMyNodeActor* AMyNodeActor::CreateAndSubscribeNewNode()
{
	/*
		При создании ноды смещаем её на 200X 
	 */
	
	UWorld* World = GetWorld();

	if (World)
	{
		AMyNodeActor* NewNode = Cast<AMyNodeActor>(World->SpawnActor(AMyNodeActor::StaticClass()));
		
		//NewNode->SubDel.AddDynamic(this, &AMyNodeActor::CallEvent);

		//ISubscription.Add(NewNode);

		//todo
		//NewNode->Mesh->OverlapComponent(NewNode->GetActorLocation(), FQuat(), FCollisionShape());

		return NewNode;
	}
	
	return nullptr;
}

/*
void AMyNodeActor::EventSubscribeOnNode()
{

	int32 my = ISubscription.Num();
	int32 del = OnMeSubscription.Num();

	if (my && del)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Ноде пора на помойку а не подписки подписывать"));
		return;
	}

	int32 indexsusa = FMath::Rand() % (my + del);

	AMyNodeActor* newsub = nullptr;

	if (indexsusa < my && my != 0)
	{
		
		int32 indexmysub = indexsusa == my ? indexsusa - 1 : indexsusa;

		newsub = ISubscription[indexmysub];

		int32 mysub_nei = newsub->ISubscription.Num();
		int32 del_nei = newsub->OnMeSubscription.Num();
		int32 indexsusa_nei = FMath::Rand() % (mysub_nei + del_nei);

		if (indexsusa_nei < mysub_nei && mysub_nei != 0)
		{
			int32 indexmysub_nei = indexsusa_nei == mysub_nei ? indexsusa_nei - 1 : indexsusa_nei;
			
			newsub = newsub->ISubscription[indexmysub_nei];

			check(newsub);
		}
		else if (del_nei != 0)
		{
			int32 inxdel = indexsusa_nei - mysub_nei;

			newsub = newsub->OnMeSubscription[inxdel];

			check(newsub);
		}
	
	}
	else if (del != 0)
	{
		int32 inxdel = indexsusa - my;
		
		newsub = OnMeSubscription[inxdel];

		int32 mysub_nei = newsub->ISubscription.Num();
		int32 del_nei = newsub->OnMeSubscription.Num();
		int32 indexsusa_nei = FMath::Rand() % (mysub_nei + del_nei);

		if (indexsusa_nei < mysub_nei && mysub_nei != 0)
		{
			int32 indexmysub_nei = indexsusa_nei == mysub_nei ? indexsusa_nei - 1 : indexsusa_nei;

			newsub = newsub->ISubscription[indexmysub_nei];

			check(newsub);
		}
		else if (del_nei != 0)
		{
			inxdel = indexsusa_nei - mysub_nei;

			newsub = newsub->OnMeSubscription[inxdel];

			check(newsub);
		}
		
	}
	
	
	if (OnMeSubscription.Find(newsub))
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Узел был в подписках!"));
		return;
	}
	

	if (ISubscription.Find(newsub))
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Мы были подписаны на узел!"));
		return;
	}

	// Ура подписка удолась !!!

	ISubscription.Add(newsub);

	newsub->SubscribeOnMe(this);
	
}
*/

#pragma endregion 

