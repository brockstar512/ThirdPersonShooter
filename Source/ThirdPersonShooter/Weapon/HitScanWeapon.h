// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class THIRDPERSONSHOOTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override;

protected:
	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);
	void WeaponTraceHit(const FVector& TraceStart, const FVector& TraceEnd, FHitResult& OutHit);
	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;
	UPROPERTY(EditAnywhere)
	USoundCue* HitSound;
	UPROPERTY(EditAnywhere)
	float Damage = 25.f;
private:
	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;
	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;	

	/*
	* Trace end with Scatter
	*/
	UPROPERTY(EditAnywhere, Category ="Weapon Scatter")
	float DistanceToSphere = 800.f;
	UPROPERTY(EditAnywhere, Category ="Weapon Scatter")
	float SphereRadius = 75.f;
	UPROPERTY(EditAnywhere, Category ="Weapon Scatter")
	bool bUseScatter = false;



};
