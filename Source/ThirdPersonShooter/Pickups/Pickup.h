// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class THIRDPERSONSHOOTER_API APickup : public AActor
{
	GENERATED_BODY()
	
public:
	APickup();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 45.f;

private:

	UPROPERTY(EditAnywhere)
	class USphereComponent* OverlapSphere;

	UPROPERTY(EditAnywhere)
	class USoundCue* PickupSound;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PickupMesh;

	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* PickupEffectComponent;
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PickupEffect;

	//these give the collision time to bind to the destroy delegate.
	//otherwise if spawned when character is over it it will be destroyed before
	//the destroy function can bind
	FTimerHandle BindOverlapTimer;	
	float BindOverlapTime = 0.25f;
	void BindOverlapTimerFinished();

};

