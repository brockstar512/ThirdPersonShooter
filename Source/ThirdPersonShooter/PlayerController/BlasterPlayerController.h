// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class THIRDPERSONSHOOTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);

	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDGrenades(int32 Grenades);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float time);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void OnMatchStateSet(FName State);
	void HandleCoolDown();

protected:
	virtual float GetServerTime();
	virtual void BeginPlay() override;
	virtual void ReceivedPlayer() override;//sync with server clock as soon as possible.
	void SetHUDTime();
	void PollInit();
	void HandleMatchHasStarted();
	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();
	//sync time between client and server
	//requests current server time passing in clients time when the request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);
	//reports the current server tine to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);
	//difference in client and server time;
	float ClientServerDelta = 0;
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;
	float TimeSyncRunningTime = 0;
	void CheckTimeSync(float DeltaTime);
	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);
private:
    UPROPERTY()
	class ABlasterHUD* BlasterHUD;
	float LevelStartingTime = 0;
	float MatchTime = 0.0f;
	float WarmUpTime = 0.f;
	float CoolDownTime = 0.f;
	uint32 CountdownInt = 0;
	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;
	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	bool bInitializeHealth = false;
	bool bInitializeShield = false;
	bool bInitializeScore = false;
	bool bInitializeGrenades = false;
	bool bInitializeDefeats = false;
	bool bInitializeCarriedAmmo = false;
	bool bInitializeWeaponAmmo = false;


	float HUDHealth;
	float HUDMaxHealth;
	float HUDShield;
	float HUDMaxShield;
	int32 HUDDefeats;
	float HUDScore;
	int32 HUDGrenades;
	float HUDCarriedAmmo;
	float HUDWeaponAmmo;

};
