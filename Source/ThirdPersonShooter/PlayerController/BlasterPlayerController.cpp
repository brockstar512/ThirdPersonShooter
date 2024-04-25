// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"
#include "ThirdPersonShooter/HUD/BlasterHUD.h"
#include "ThirdPersonShooter/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "ThirdPersonShooter/Character/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "ThirdPersonShooter/GameMode/BlasterGameMode.h"
#include "ThirdPersonShooter/HUD/Announcement.h"
#include "Kismet/GameplayStatics.h"


float ABlasterPlayerController::GetServerTime()
{
	if(HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return  GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABlasterPlayerController::BeginPlay()
{
    Super::BeginPlay();
	
    BlasterHUD = Cast<ABlasterHUD>(GetHUD());

	ServerCheckMatchState();

}
void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABlasterPlayerController,MatchState);
}


void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if(IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
	
}



void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
    BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

    if(BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->HealthBar && BlasterHUD->CharacterOverlay->HealthText)
    {
        const float HealthPercent = Health / MaxHealth;
        BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
        FString HealthText = FString::Printf(TEXT("%d/%d"),FMath::CeilToInt(Health),FMath::CeilToInt(MaxHealth));
        BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
    }   
	else 
	{
		bInitializeCharacterOverlay = true;

		//cache these values until we get the overlay
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
    BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

    if(BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->ScoreAmount)
    {
        FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
        BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else {
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
	}
}
void ABlasterPlayerController::SetHUDDefeats(int32 Defeats)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		BlasterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else {
		bInitializeCharacterOverlay = true;
		HUDDefeats = Defeats;
	}
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
    BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount;
        
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	    BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount;
        //UE_LOG(LogTemp, Warning, TEXT("updated carried ammo! %d"), Ammo);

	if (bHUDValid)
	{
		FString CarriedAmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmoText));
	}
}

void ABlasterPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->MatchCountDownText;
	if (bHUDValid)
	{
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->CharacterOverlay->MatchCountDownText->SetText(FText::FromString(CountdownText));
	}
}

void ABlasterPlayerController::SetHUDAnnouncementCountdown(float time)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->Announcement &&
		BlasterHUD->Announcement->WarmUpTime;
	if (bHUDValid)
	{
		int32 Minutes = FMath::FloorToInt(time / 60.f);
		int32 Seconds = time - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->Announcement->WarmUpTime->SetText(FText::FromString(CountdownText));
	}
}


void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if (BlasterCharacter)
	{
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
	}
}

void ABlasterPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = WarmUpTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::InProgress)
	{
		TimeLeft = WarmUpTime + MatchTime -  GetServerTime() + LevelStartingTime;
	}

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	if (CountdownInt != SecondsLeft)
	{
		//SetHUDMatchCountdown(MatchTime - GetWorld()->GetTimeSeconds());
		//SetHUDMatchCountdown(MatchTime - GetServerTime());
		if (MatchState == MatchState::WaitingToStart)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}

	CountdownInt = SecondsLeft;
}

void ABlasterPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (BlasterHUD && BlasterHUD->CharacterOverlay)
		{
			CharacterOverlay = BlasterHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				SetHUDHealth(HUDHealth,HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);
			}
		}
	}
}



void ABlasterPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime +=DeltaTime;
	if(IsLocalPlayerController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime =0.f;
	}
}


void ABlasterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;


	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
		

	}
}

//this will be called by the server when the variable changes
void ABlasterPlayerController::OnRep_MatchState()
{

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
		/*
		* the server changes the variable... when the variable changes the client function OnRepMatchState is called
		* 
		* telling server to register this variable
		*DOREPLIFETIME(ABlasterPlayerController,MatchState); 
		* 
		* the server changes the variable... then this is saying run this function when the variable is replicated
		*UPROPERTY(ReplactedUsing =  OnRep_MatchState);
		*FName MatchState;
		
		*function to run
		*UFUNCTION();
		*void OnRep_MatchState();
		*/

	}

}


void ABlasterPlayerController::HandleMatchHasStarted()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		BlasterHUD->AddCharacterOverlay();
		if (BlasterHUD->Announcement)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));

	if (GameMode)
	{
		 MatchTime = GameMode->MatchTime;
		 WarmUpTime = GameMode->WarmupTime;
		 LevelStartingTime = GameMode->LevelStartingTime;
		 MatchState = GameMode->GetMatchState();

		 ClientJoinMidgame_Implementation(MatchState, WarmUpTime, MatchTime, LevelStartingTime);

		 
		 if (BlasterHUD && MatchState==MatchState::WaitingToStart)
		 {
			 BlasterHUD->AddAnnouncement();
		 }
	}
}
void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeOfServerRecieved)
{
	//calculate roundtrip time
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	//round trip time is time of there and back again... current server time is the time the server recieved 
	//the messeage and the half the round trip time projected into the future
	float CurrentServerTime = TimeOfServerRecieved + (0.5f * RoundTripTime);
	//the difference between time of client and server time
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void ABlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReciept = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReciept);
}

void ABlasterPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float WarmUp, float Match, float StartingTime)
{
	 LevelStartingTime = StartingTime;
	 MatchTime = Match;
	 WarmUpTime = WarmUp;
	 MatchState = StateOfMatch;
	 OnMatchStateSet(MatchState);


	 if (BlasterHUD && MatchState == MatchState::WaitingToStart)
	 {
		 BlasterHUD->AddAnnouncement();
	 }
	
}
