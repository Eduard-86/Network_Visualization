// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ActorComponent.h"


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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateNode, float, Val, class AMyNodeActor*, Who);

class UShapeComponent;
class AMyNetwork;

UCLASS()
class WILDWORLD_API AMyNodeActor : public AActor
{
	GENERATED_BODY()

	friend class AMyNetwork;

//public:	
	// Sets default values for this actor's properties
	AMyNodeActor();

	FDelegateNode SubDel;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UShapeComponent* Trigger;

	//TArray<AActor*> ArrayNodeOnTheWorld;
	
	UMaterialInstanceDynamic* DynMater;

	// ������� ��� �������� 
	UFUNCTION()
	void CallEvent(float val, AMyNodeActor* who);

	// event for the collision,
	UFUNCTION()
	void BrotcastEvents(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void SubscribeOnMe(AMyNodeActor* NewSubNode);

	void UnSubscribeOnMe(AMyNodeActor* NewSubNode);
	
#pragma region Events

	TArray<AMyNodeActor*> OnMeSubscription;

	TArray<AMyNodeActor*> ISubscription;

	void BroatcastEvectsAllSubs(); 

	void EventSubscribeOnNode(); 
	
	void UnSubscribe(AMyNodeActor* NewSubNode); 
	
	AMyNodeActor* CreateAndSubscribeNewNode(); 

	// �� �����������
	UFUNCTION()
	void Inaction(){};
	
#pragma endregion 

};

