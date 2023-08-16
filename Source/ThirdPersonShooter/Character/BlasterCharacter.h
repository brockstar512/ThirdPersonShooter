// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ThirdPersonShooter/BlasterTypes/TurningInPlace.h"
#include "ThirdPersonShooter/MyInterfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "BlasterCharacter.generated.h"

UCLASS()
class THIRDPERSONSHOOTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	void PlayElimMontage();
	// UFUNCTION(NetMulticast, Unreliable)//this is for hit reactions which will happen often and is just sugar coating so its not an important rpc
	// void MulticastHit();
    virtual void OnRep_ReplicatedMovement() override;
	void Elim();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();
	virtual void Destroyed() override;
protected:
	virtual void BeginPlay() override;
	virtual void Jump() override;
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void AimOffset(float DeltaTime);
	void SimProxiesTurn();
	void FireButtonPressed();
	void FireButtonReleased();
	void PlayHitReactMontage();
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	void UpdateHUDHealth();
	//poll for any relevant classes and initialize our hud
	void PollInit();
private:
	void CalculateAO_Pitch();
	UPROPERTY(VisibleAnywhere, Category = CameraAnywhere)
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, Category = CameraAnywhere)
	class UCameraComponent* FollowCamera;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))//allowing acccess to a private variable
	class UWidgetComponent* OverheadWidget;
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);//can only have a parameter of type it is replicating...this is to help figure out which widget to hide when we are not with it anymore
	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* Combat;
	//reliable are guaranteed to be executed unreliable may or not be executed depending if the data packet was dropped similar to tcp vs UDP
	UFUNCTION(Server,Reliable)//one off actiosna re good to make reliable
	void ServerEquipButtonPressed();
	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;	
	FRotator StartingAimRotation;
	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ElimMontage;
	void HideCameraIfCharacterClose();
	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CalculateSpeed();

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;
	UPROPERTY(ReplicatedUsing = OnRep_Health,VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;
	UFUNCTION()
	void OnRep_Health();
	bool bElimmed = false;
	FTimerHandle ElimTimer;
	UPROPERTY(EditDefaultsOnly)//can only edit the default character
	float ElimDelay = 3.f;
	void ElimTimerFinished();
	/*
	Disolve effect
	*/
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat DissolveTrack;
	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;
	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();
	//dynamic instance that changes
	UPROPERTY(VisibleAnywhere,Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;
	//material instance that is the reference to the material
	UPROPERTY(EditAnywhere,Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;

	class ABlasterPlayerController* BlasterPlayerController;
//getters and setters
UPROPERTY(EditAnywhere)
float CameraThreshold = 200.f;
//when the replicated variable changes the inline function is going to run on the client
//replication only changes when the variable changes

/**
 * Elim Bot
*/
UPROPERTY(EditAnywhere)
UParticleSystem* ElimBotEffect;
UPROPERTY(VisibleAnywhere)
UParticleSystemComponent* ElimBotComponent;
UPROPERTY(EditAnywhere)
class USoundCue* ElimBotSound;
UPROPERTY()
class ABlasterPlayerState* BlasterPlayerState;

public:	
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	FORCEINLINE float GetAO_Yaw() const {return AO_Yaw;}
	FORCEINLINE float GetAO_Pitch() const {return AO_Pitch;}
	AWeapon* GetEqippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const {return TurningInPlace;}
	//UPROPERTY(VisibleAnywhere, Category = CombatComponent)this will add a vriable string  called hello to the category on myblastercharacter
	//FString hello = "Hello";
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const {return bRotateRootBone;}
	FORCEINLINE bool IsElimmed() const {return bElimmed;}
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
};
