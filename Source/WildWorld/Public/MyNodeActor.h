// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ActorComponent.h"

#include "Chaos/Pair.h"


#include "MyNodeActor.generated.h"

/*
	Ну чтож

	Требуется восоздать сеть узлов
	Нужен класс узла "данный класс" и менреджер это гейм инстанс

	Что требуется в этом классе

		// call event on our subscribers 
		void CallEvent();

		// method subscribe on the node
		void SubscribeOnNode();

		// события отписки от того на кого подписан
		void UnSubscribe();

		Node * CreateAndSubscribeNewNode();

		void Inaction();
 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateNode, float, Val, class AMyNodeActor*, Who);

USTRUCT(BlueprintType)
struct FSubData
{
	GENERATED_BODY()

	
	FSubData()
	{
		SubNode = nullptr;
		SubType = false;
		SubValue = 0;
	};
	
	
	FSubData(AMyNodeActor* Sub, bool Type, int Val = 0)
		: SubNode(Sub), SubType(Type), SubValue(Val)
	{};

	AMyNodeActor* SubNode;
	bool SubType;
	int SubValue;
	
};

class UShapeComponent;
class AMyNetwork;

UCLASS()
class WILDWORLD_API AMyNodeActor : public AActor
{
	GENERATED_BODY()

	friend class AMyNetwork;


#pragma region AuxiliaryLogic

//public:	
	// Sets default values for this actor's properties
	AMyNodeActor();

	

protected:
	
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UShapeComponent* Trigger;

	//TArray<AActor*> ArrayNodeOnTheWorld;
	
	UMaterialInstanceDynamic* DynMater;


#pragma endregion 

	FDelegateNode SubDel;
	
	// функция для подписки 
	UFUNCTION()
	void SumEvent(float val, AMyNodeActor* who);

	// функция для подписки 
	UFUNCTION()
	void CounterEvent(float val, AMyNodeActor* who);

	// event for the collision,
	UFUNCTION()
	void BrotcastEvents(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void SubscribeOnMe(AMyNodeActor* NewSubNode);

	void UnSubscribeOnMe(AMyNodeActor* NewSubNode);
	
#pragma region Events

	// Те на кого я подписан
	//TMap<AMyNodeActor*, Chaos::Pair<bool, float>> MySubscription1;

	TArray<FSubData> MySubscription;
	
	//TArray<AMyNodeActor*> MySubscription;

	//TArray<AMyNodeActor*> ISubscription;

	void BroatcastEvectsAllSubs(); 

	void EventSubscribeOnNode(); 
	
	void UnSubscribe(AMyNodeActor* NewSubNode); 
	
	AMyNodeActor* CreateAndSubscribeNewNode(); 

	// Мб пригодиться
	UFUNCTION(BlueprintCallable)
	virtual void Inaction(){};
	
#pragma endregion 

};

