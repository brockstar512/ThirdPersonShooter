// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

UCLASS()
class THIRDPERSONSHOOTER_API ACasing : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACasing();
private:
UPROPERTY(EditAnywhere)
float ShellEjectionImpulse;
UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CasingMesh;
	//varibales and functions that are not going to change stick to private
	UPROPERTY(EditAnywhere)
	class USoundCue* ShellSound;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:	


};
