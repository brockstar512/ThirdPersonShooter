// Fill out your copyright notice in the Description page of Project Settings.

//this should match pickupspawnpoint but instead of destroy it shoudl be bound to euqip or pick.

#include "WeaponSpawnPoint.h"
#include "Weapon.h"

// Sets default values
AWeaponSpawnPoint::AWeaponSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	//create a rootcomponent
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SpawnMarker = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnMarker"));
	SpawnMarker->SetupAttachment(RootComponent);
}

void AWeaponSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority() && WeaponClass)
	{
		SpawnPickup();
	}
}

void AWeaponSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AWeaponSpawnPoint::SpawnPickup()
{
	if (SpawnMarker)
	{

		FTransform SpawnTransform = SpawnMarker->GetComponentTransform();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		SpawnedPickup = GetWorld()->SpawnActor<AWeapon>(
			WeaponClass,
			SpawnTransform,
			SpawnParams
		);

	}
}

