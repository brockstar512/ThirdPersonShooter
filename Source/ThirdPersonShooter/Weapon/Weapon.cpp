// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);//block all channels?
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn,ECollisionResponse::ECR_Ignore);//except pawwn?
	//The Pawn is the physical representation of a player within the world
	
	// 	An Actor is something you place in your world.
	// A Pawn is a special Actor because Controllers(= Humans and AI) can possess them and feed input to them.
	//Pawn: A Pawn is an Actor that can be an “agent” within the world. Pawns can be possessed by a Controller, they are set up to easily accept input, and they can do various and sundry other player-like things. Note that a Pawn is not assumed to be humanoid.

	//Character: A Character is a humanoid-style Pawn. It comes with a CapsuleComponent for collision and a CharacterMovementComponent by default. It can do basic human-like movement, it can replicate movement smoothly across the network, it has some animation-related functionality.
	
	//turn it off until we pick it up
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	//region which they can pick up the weapon
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere -> SetupAttachment(RootComponent);
	//we only want the server to detect this
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	//we are only enabling collision on the server
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);


}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	//if we are the server enable collision//GetLocalRole() == ENetRole::ROLE_Authority)
	if(HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn,ECollisionResponse::ECR_Overlap);
	}
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

