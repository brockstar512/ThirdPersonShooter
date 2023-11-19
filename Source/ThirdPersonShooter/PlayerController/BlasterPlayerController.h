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
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
protected:
	virtual float GetServerTime();
	virtual void BeginPlay() override;
	virtual void ReceivedPlayer() override;//sync with server clock as soon as possible.
	void SetHUDTime();
	/*
	sync time between client and server
	*/
	//requests current server time passing in clients time when the request was sent
	UFUNCTION(Server,Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);
	//reports the current server tine to the client in response to ServerRequestServerTime
	UFUNCTION(Client,Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeOfServerRecieved);
	//difference in client and server time;
	float ClientServerDelta = 0;
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;
	float TimeSyncRunningTime = 0;
	void CheckTimeSync(float DeltaTime);
private:
    UPROPERTY();
	class ABlasterHUD* BlasterHUD;
	float MatchTime = 120.f;
	uint32 CountdownInt = 0;
};
