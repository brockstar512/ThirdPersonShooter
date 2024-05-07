// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketMovementComponent.h"

//in the header file you are inside the class.. in the the cpp file we are out of the class so we need to define where the enum is coming from.. if it was outside of the class but in the header file we woul dhave access to ti by just includeing the header file
URocketMovementComponent::EHandleBlockingHitResult URocketMovementComponent::HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);
	//if you hit something just adavnced to the next update frme
	return EHandleBlockingHitResult::AdvanceNextSubstep;
}

void URocketMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	// Rockets should not stop; only explode when their CollisionBox detects a hit
}