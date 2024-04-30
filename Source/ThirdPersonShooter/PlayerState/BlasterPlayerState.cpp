// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"
#include "ThirdPersonShooter/Character/BlasterCharacter.h"
#include "ThirdPersonShooter/PlayerController/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerState, Defeats);
}

void ABlasterPlayerState::OnRep_Score()
{
    Super::OnRep_Score();

    Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;

    if(Character)
    {
        Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
        
        if(Controller)
        {
            Controller->SetHUDScore(GetScore());
        }
    }
}

void ABlasterPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character && Character->Controller)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore()+ ScoreAmount);
    Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;

    if(Character)
    {
        Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
        
        if(Controller)
        {
            Controller->SetHUDScore(Score);
        }
    }
}

void ABlasterPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	//UE_LOG(LogTemp, Warning, TEXT("stage 0"));

	Defeats += DefeatsAmount;
	// UE_LOG(LogTemp, Warning, TEXT("total defeats %d"),Defeats);
	// UE_LOG(LogTemp, Warning, TEXT("DefeatsAmount  %d"),DefeatsAmount);



	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;

	// UE_LOG(LogTemp, Warning, TEXT("char %s"), ( Character == nullptr ? TEXT("missing") : TEXT("found") ));
	// UE_LOG(LogTemp, Warning, TEXT("char controller %s"), ( Character->Controller == nullptr ? TEXT("missing") : TEXT("found") ));

					//Character->Controller is often missing
					//i took out  && Character->Controller
	if (Character)
	{
			//UE_LOG(LogTemp, Warning, TEXT("stage 1"));

		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			//UE_LOG(LogTemp, Warning, TEXT("stage 2"));

			// UE_LOG(LogTemp, Warning, TEXT("defeats added %d"),Defeats);
			// UE_LOG(LogTemp, Warning, TEXT("DefeatsAmount added %d"),DefeatsAmount);

			Controller->SetHUDDefeats(Defeats);
		}
	}
}