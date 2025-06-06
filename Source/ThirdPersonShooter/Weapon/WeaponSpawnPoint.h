// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponSpawnPoint.generated.h"

UCLASS()
class THIRDPERSONSHOOTER_API AWeaponSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponSpawnPoint();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY()
	class AWeapon* SpawnedPickup;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void APickupSpawnPoint();
	void SpawnPickup();
	void SpawnPickupTimerFinished();



private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> WeaponClass;

	FTimerHandle SpawnPickupTimer;


	UPROPERTY(EditAnywhere)
	float SpawnPickupTime;

};
