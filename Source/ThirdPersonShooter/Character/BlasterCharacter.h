// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ThirdPersonShooter/BlasterTypes/TurningInPlace.h"
#include "ThirdPersonShooter/MyInterfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "ThirdPersonShooter/BlasterTypes/CombatState.h"
#include "BlasterCharacter.generated.h"

UCLASS()
class THIRDPERSONSHOOTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	/*
	* APlayerController vs ACharacter
	If you want to implement some complex input functionality (for example if there are multiple players on one game client or there is a need to change characters dynamically at runtime), it’s better (and sometimes necessary) to use PlayerController. In this case PlayerController handles input and the issues commands to the Pawn.
	For example, in deathmatch games Pawn may change during gameplay, but PlayerController usually remains the same.
	Character class represents player in the game world. It provides functionality for animation, collision, movement and basic networking and input modes. Thus, if your input is not complicated and there is no need to change character dynamically at runtime, Character class is more suitable. For example, you can use it in single player first-person shooter.
	player cotroller is the interface between the pawn and you controlling the player */

	//blaster character manages the player input 
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayReloadMontage();
	void PlayFireMontage(bool bAiming);
	void PlayElimMontage();
	// UFUNCTION(NetMulticast, Unreliable)//this is for hit reactions which will happen often and is just sugar coating so its not an important rpc
	// void MulticastHit();
    virtual void OnRep_ReplicatedMovement() override;
	void Elim();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();
	virtual void Destroyed() override;
	void PlayThrowGrenadeMontage();

	//this will only run on blue prints. a cpp function body will break compilation.there is a way to run both the C++ implementation and the Blueprint logic
	// by using the BlueprintNativeEvent macro argumentIf the Blueprint overrides it, the Blueprint version is called instead, not both automatically. But you can explicitly call the C++ version from Blueprint
	//
	void UpdateHUDHealth();
	void UpdateHUDShield();
	void UpdateHUDAmmo();
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);
	void SpawnDefaultWeapon();
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
	void ReloadButtonPressed();
	void AimButtonReleased();
	void AimOffset(float DeltaTime);
	void SimProxiesTurn();
	void FireButtonPressed();
	void FireButtonReleased();
	void PlayHitReactMontage();
	void RotateInPlace(float DeltaTime);
	void GrenadeButtonPressed();
	

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;
	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* Buff;

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
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ThrowGrenadeMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;
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
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 100.f;
	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = "Player Stats")
	float Shield = 0.f;
	UFUNCTION()
	void OnRep_Health(float LastHealth);
	UFUNCTION()
	void OnRep_Shield(float LastShield);
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


	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;
	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;
	UPROPERTY(EditAnywhere)
	class USoundCue* ElimBotSound;
	UPROPERTY()
	class ABlasterPlayerState* BlasterPlayerState;
	/*
	grenade
	*/
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* AttachedGrenade;

	//default weapon
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;
public:	
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	FORCEINLINE float GetAO_Yaw() const {return AO_Yaw;}
	FORCEINLINE float GetAO_Pitch() const {return AO_Pitch;}
	AWeapon* GetEqippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const {return TurningInPlace;}
	//UPROPERTY(VisibleAnywhere, Category = CombatComponent)this will add a vriable string  called hello to the caFtegory on myblastercharacter
	//FString hello = "Hello";
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const {return bRotateRootBone;}
	FORCEINLINE bool IsElimmed() const {return bElimmed;}
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	ECombatState GetCombatState() const;
	UPROPERTY(Replicated)
	bool bDisableGameplay = false;
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }
	FORCEINLINE bool GetDisabledGameplay() const { return bDisableGameplay; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	FORCEINLINE UBuffComponent* GetBuff() const { return Buff; }

};
