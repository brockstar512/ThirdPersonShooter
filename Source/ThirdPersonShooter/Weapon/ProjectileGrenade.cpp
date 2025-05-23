// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileGrenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
//the movement of projectiles is controlled in the projectile movement component
AProjectileGrenade::AProjectileGrenade()
{
	/*
	To make a projectile in Unreal Engine 5 follow a higher arc without affecting its overall speed, you need to adjust the initial launch angle or velocity direction, rather than increasing the speed itself. You can do that in projectile movement comppnent

	//i increaces the z velocity in blueprint, but I could do it in code instead, but make sure to do it in combat so it does not effect grenade launcher
	*/

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->bShouldBounce = true;

}

void AProjectileGrenade::Destroyed()
{
	ExplodeDamage();
	Super::Destroyed();
}

void AProjectileGrenade::BeginPlay()
{
	//we are not going to begin play because the parent class explodes on impact. we want to explode after timer
	//so we are going to use the base class override
	AActor::BeginPlay();
	SpawnTrailSystem();
	StartDestroyTimer();
	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce);
}

void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if (BounceSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BounceSound, GetActorLocation());

	}
}
