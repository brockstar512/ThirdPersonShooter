// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "ThirdPersonShooter/Character/BlasterCharacter.h"
#include "ThirdPersonShooter/ThirdPersonShooter.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundCue.h"
// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	//send a raycast to hit this channel
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	//block static meshes
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	//if you hit the skeletal mesh block it
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);

}

void AProjectile::Destroyed()
{
	Super::Destroyed();
	//because when you destroy a replicated actor every machine where that actor is is also going to be destroyed this will run for everyone
	if(ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ImpactParticles,
			GetActorTransform()
		);
	}
	if(ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound,GetActorLocation());
	}
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	if(Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			CollisionBox,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
	}

	if(HasAuthority())
	{
		//UE_LOG(LogTemp, Warning, TEXT("binding on hit function"));

		//we are only applying hit events to server
		CollisionBox->OnComponentHit.AddDynamic(this,&AProjectile::OnHit);
		
	}

}

void AProjectile::StartDestroyTimer()
{

	//destroy the effects after x amount of time
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&AProjectile::DestroyTimerFinished,
		DestroyTime
	);
}

void AProjectile::DestroyTimerFinished()
{
	Destroy();
}

void AProjectile::OnHit(UPrimitiveComponent * HitComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit)
{

	//UE_LOG(LogTemp, Warning, TEXT("On Hit"));


	Destroy();
}

void AProjectile::ExplodeDamage()
{

	//apply radial damage
	//what was the pawn who owns the weapon and fired the weapon
	APawn* FiringPawn = GetInstigator();

	if (FiringPawn && HasAuthority())
	{
		//if this was fire by a controller
		AController* FiringController = FiringPawn->GetController();

		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,//world context object
				Damage, //base damage
				10.f, // minimum damage
				GetActorLocation(), //origin
				DamageInnerRadius,
				DamageOuterRadius, //Damage Outer radius
				1.f, //Damage Falloff
				UDamageType::StaticClass(), //Damage Tpe
				TArray<AActor*>(), //ingore Actors (friendly fire)
				this, //damage causer 
				FiringController //instigatorController
			);
		}
	}
}

void AProjectile::SpawnTrailSystem()
{
	if (TrailSystem)
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

