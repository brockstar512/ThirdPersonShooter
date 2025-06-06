// Fill out your copyright notice in the Description page of Project Settings.

//this should match pickupspawnpoint but instead of destroy it shoudl be bound to euqip or pick.

#include "WeaponSpawnPoint.h"

// Sets default values
AWeaponSpawnPoint::AWeaponSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AWeaponSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWeaponSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

