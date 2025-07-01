// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LagCompensationComponent.generated.h"


USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()
		UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;
};

UCLASS()
class THIRDPERSONSHOOTER_API ALagCompensationComponent : public AActor
{
	GENERATED_BODY()
	
public:	
	ALagCompensationComponent();
	friend class ABlasterCharacter;
	virtual void Tick(float DeltaTime) override;



protected:
	virtual void BeginPlay() override;

private:
UPROPERTY()
ABlasterCharacter* Character;

UPROPERTY()
class ABlasterPlayerController* Controller; 

};
