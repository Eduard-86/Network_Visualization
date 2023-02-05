// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ActorComponent.h"

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

class UShapeComponent;
class AMyNetwork;


UENUM(BlueprintType)
enum class ESubType : uint8
{
	Sum = 0,
	Counter = 1,
};

USTRUCT(BlueprintType)
struct FSubData
{
	GENERATED_BODY()

	
	FSubData()
	{
		SubNode = nullptr;
		SubType = ESubType::Sum;
		SubValue = 0;
	};
	
	
	FSubData(AMyNodeActor* Sub, ESubType Type, int Val = 0)
		: SubNode(Sub), SubType(Type), SubValue(Val)
	{};

	AMyNodeActor* SubNode;
	ESubType SubType;
	int SubValue;

	bool operator == (FSubData a)
	{
		return this->SubNode == a.SubNode;
	}
	
};



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

	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	//UShapeComponent* Trigger;
	
	UMaterialInstanceDynamic* DynMater;

	// Value all events Sums
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float SumAllEventValue = 0;
	// Value all events Counters
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float CounterAllEventValue = 0;

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
	void OverlapBrotcastEvents(UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor,	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
	bool bFromSweep, const FHitResult& SweepResult);

	void SubscribeOnMe(AMyNodeActor* NewSubNode, ESubType SubType);

	void UnSubscribeOnMe(AMyNodeActor* NewSubNode, ESubType SubType);

	bool IsEmpty()
	{
		return !(SubDel.GetAllObjects().Num() || MySubscription.Num());
	};

	UFUNCTION(BlueprintImplementableEvent)
	void WidgetAP(float val, ESubType eventtype);
	
#pragma region Events

	// Я подписан на 
	TArray<FSubData> MySubscription;
	
	void BroadcastEventsAllSubs(); 

	void SubscribeOnNode(); 
	
	void UnSubscribe(); 
	
	void SubscribeOnNewCreatedNode(AMyNodeActor* NodeOnSub);

	void SubscribeOnNewCreatedNode(AMyNodeActor* NodeOnSub, ESubType SubType);

	// for future
	UFUNCTION(BlueprintCallable)
	virtual void Inaction(){};
	
#pragma endregion

#pragma region Visual

protected:

	float HeightHeirLocation;

	float WidthHeirLocation;

#pragma endregion

};

