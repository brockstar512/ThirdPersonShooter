// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "ThirdPersonShooter/Character/BlasterCharacter.h"
#include "ThirdPersonShooter/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

void ABlasterGameMode::PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController,class ABlasterPlayerController* AttackerController)
{
    if(ElimmedCharacter)
    {
        ElimmedCharacter->Elim();
    }
}

void ABlasterGameMode::RequestRespawn(ACharacter * ElimmedCharacter, AController * ElimmedController)
{
    if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}
