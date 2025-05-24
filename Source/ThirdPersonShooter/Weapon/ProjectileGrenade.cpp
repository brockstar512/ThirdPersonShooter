// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileGrenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
//the movement of projectiles is controlled in the projectile movement component
AProjectileGrenade::AProjectileGrenade()
{
	/*

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


	if (GEngine)
	{
		const FVector Velocity = GetVelocity();
		const FString NetRole = HasAuthority() ? TEXT("Server") : TEXT("Client");
		const FString Message = FString::Printf(TEXT("[%s] Velocity: %s"), *NetRole, *Velocity.ToString());

		GEngine->AddOnScreenDebugMessage(
			-1,              // Show new message each frame
			5.0f,            // Short duration so it refreshes each frame
			FColor::Cyan,    // Color
			Message
		);
	}


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
