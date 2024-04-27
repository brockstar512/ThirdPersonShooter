// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

/**
 * 
 */

namespace MatchState
{
	extern const FName Cooldown;//match duration has been reachecd. display winner and begind cooldown
}
UCLASS()
class THIRDPERSONSHOOTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()
public:
ABlasterGameMode();
virtual void Tick(float DeltaTime) override;
virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController,class ABlasterPlayerController* AttackerController);
virtual void RequestRespawn(class ACharacter* ElimmedCharacter, AController* ElimmedController);

//edittable from the blueprints
UPROPERTY(EditDefaultsOnly)
float WarmupTime = 10.f;
UPROPERTY(EditDefaultsOnly)
float CooldownTime = 10.f;
UPROPERTY(EditDefaultsOnly)
float MatchTime = 120.f;

float LevelStartingTime = 0.f;
FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;
	


};
