// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ActorComponent.h"
#include <utility>


#include "MyNodeActor.generated.h"

/*
	�� ����

	��������� ��������� ���� �����
	����� ����� ���� "������ �����" � ��������� ��� ���� �������

	��� ��������� � ���� ������

		// call event on our subscribers 
		void CallEvent();

		// method subscribe on the node
		void SubscribeOnNode();

		// ������� ������� �� ���� �� ���� ��������
		void UnSubscribe();

		Node * CreateAndSubscribeNewNode();

		void Inaction();
 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateNode, int, Val, class AMyNodeActor*, Who);

class UShapeComponent;
class AMyNetwork;

UCLASS()
class WILDWORLD_API AMyNodeActor : public AActor
{
	GENERATED_BODY()

	friend class AMyNetwork;

public:	
	// Sets default values for this actor's properties
	AMyNodeActor();

	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UShapeComponent* Trigger;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FDelegateNode SubDel;
	 
	UMaterialInstanceDynamic* DynMater;

	
	//TArray<AMyNodeActor*> OnMeSubscription;
	//TArray<AMyNodeActor*> ISubscription;

	TArray< std::pair<AMyNodeActor*, bool>> OnMeSubscription;

	TMap<AMyNodeActor*, std::pair<bool, int>> ISubscription;


	// event for the collision,
	UFUNCTION()
	void BroadcastEvents(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void SubscribeOnMe(AMyNodeActor* NewSubNode, bool MetKey);

	void UnSubscribeOnMe(AMyNodeActor* NewSubNode);

	// ������� ��� �������� 
	UFUNCTION()
	void SumEvent(int val, AMyNodeActor* who);

	UFUNCTION()
	void CallEvent(int val, AMyNodeActor* who);
	
#pragma region Events

	
	void BroadcastEventsAllSubs(); 

	void EventSubscribeOnNode(); 
	
	void UnSubscribe(AMyNodeActor* NewSubNode); 
	
	AMyNodeActor* CreateAndSubscribeNewNode(); 

	// �� �����������
	UFUNCTION()
	void Inaction(){};
	
#pragma endregion 

};

