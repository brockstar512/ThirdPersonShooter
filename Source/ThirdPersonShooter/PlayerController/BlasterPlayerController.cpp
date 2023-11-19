// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"
#include "ThirdPersonShooter/HUD/BlasterHUD.h"
#include "ThirdPersonShooter/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "ThirdPersonShooter/Character/BlasterCharacter.h"

float ABlasterPlayerController::GetServerTime()
{
	if(HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return  GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABlasterPlayerController::BeginPlay()
{
    Super::BeginPlay();

    BlasterHUD = Cast<ABlasterHUD>(GetHUD());
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
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
    BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

    if(BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->ScoreAmount)
    {
        FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
        BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
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
        UE_LOG(LogTemp, Warning, TEXT("updated carried ammo! %d"), Ammo);

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
	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetWorld()->GetTimeSeconds());
	if (CountdownInt != SecondsLeft)
	{
		//SetHUDMatchCountdown(MatchTime - GetWorld()->GetTimeSeconds());
		SetHUDMatchCountdown(MatchTime - GetServerTime());
	}

	CountdownInt = SecondsLeft;
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
	ClientReportServerTime(TimeOfClientRequest,ServerTimeOfReciept);
}
