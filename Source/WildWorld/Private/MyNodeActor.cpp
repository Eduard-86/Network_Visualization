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
	SubDel.AddDynamic(NewSubNode, &AMyNodeActor::CallEvent);

	OnMeSubscription.Add(NewSubNode);
}

void AMyNodeActor::UnSubscribeOnMe(AMyNodeActor* NewSubNode)
{
	check(NewSubNode);
	
	if(OnMeSubscription.Find(NewSubNode))
	{
		SubDel.RemoveDynamic(NewSubNode, &AMyNodeActor::CallEvent);
	}
	else
	{
		//todo make something error report 
	}
	
}

// main event that calleds delegate 
void AMyNodeActor::CallEvent(float val, AMyNodeActor* who)
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


#pragma region Events

void AMyNodeActor::BroatcastEvectsAllSubs()
{
	SubDel.Broadcast(FMath::Rand() % 100, this);
}


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


void AMyNodeActor::UnSubscribe(AMyNodeActor* NewSubNode)
{
	int32 size = ISubscription.Num();
	
	if(size)
	{
		int32 randind = FMath::Rand() % size;

		//ISubscription[randind]->SubDel.RemoveDynamic(this, &AMyNodeActor::CallEvent);

		NewSubNode->UnSubscribe(this);

		ISubscription.RemoveAt(randind);
		
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
		
		NewNode->SubDel.AddDynamic(this, &AMyNodeActor::CallEvent);

		ISubscription.Add(NewNode);

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

