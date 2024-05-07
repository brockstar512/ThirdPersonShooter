// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "RocketMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class THIRDPERSONSHOOTER_API URocketMovementComponent : public UProjectileMovementComponent
{
	GENERATED_BODY()
	//we are creating this and extending it from projectile moeemetn component because the itinial setting of a projecectile movement compoenent is designed to stop when it hits something
	//so we are extending it to not stop when it hits something... therefore when it hits the instgator it will not stop and the explode function will not run
	//but if it hits something else the explosion function will run so it does not matter if it stops or not because it will be destroyed. this fix is just for when the rocket
	//hits the instigator and stops bbecause we move faster than the player
	
public :
protected:
	virtual EHandleBlockingHitResult HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining) override;
	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice = 0.f, const FVector& MoveDelta = FVector::ZeroVector) override;
private:

};
