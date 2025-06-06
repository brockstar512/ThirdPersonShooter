// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};


UCLASS()
class THIRDPERSONSHOOTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();
	void Dropped();
	/**
	 * Textures for the weapon crosshairs
	*/
UPROPERTY(EditAnywhere, Category= Crosshairs)
	class UTexture2D* CrosshairsCenter;
	UPROPERTY(EditAnywhere, Category= Crosshairs)
	UTexture2D* CrosshairsLeft;
	UPROPERTY(EditAnywhere, Category= Crosshairs)
	UTexture2D* CrosshairsRight;
	UPROPERTY(EditAnywhere, Category= Crosshairs)
	UTexture2D* CrosshairsTop;
	UPROPERTY(EditAnywhere, Category= Crosshairs)
	UTexture2D* CrosshairsBottom;
	virtual void OnRep_Owner() override;
	void SetHUDAmmo();
	/** 
	* Zoomed FOV while aiming
	*/

	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	/** 
	* Automatic Fire
	*/
	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = .15f;
	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true;
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
	int32 Ammo;

	UPROPERTY(EditAnywhere)
 	class USoundCue* EquipSound;
	
	UFUNCTION()
	void OnRep_Ammo();

	void SpendRound();
	
	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

    UPROPERTY()
	class ABlasterCharacter* BlasterOwnerCharacter;
	UPROPERTY()
	class ABlasterPlayerController* BlasterOwnerController;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//need a specific signature for event collision
	//we are also making it a delegate hence the macro
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
		UFUNCTION()
	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	
private:
	UPROPERTY(VisibleAnywhere, Category="Weapon Properties" )
	USkeletalMeshComponent* WeaponMesh;
	UPROPERTY(VisibleAnywhere, Category="Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();
	//this seems just like a basic cwidget that we are namign pickup widget
	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	class UWidgetComponent* PickupWidget;
	
	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;
public:	
	void AddAmmo(int32 AmmoToAdd);
	void SetWeaponState(EWeaponState State);
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void ShowPickupWidget(bool bShowWidget);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	FORCEINLINE USphereComponent* GetAreaSphere() const {return AreaSphere;}
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const {return WeaponMesh;}
	virtual void Fire(const FVector& HitTarget);
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	bool IsEmpty();
	bool IsFull();
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	//enable or diable custom depth
	void EnableCustomDepth(bool bEnable);
	 
};

/*
relicated variables: when the variable changes on the server the same variable changes on all of the clients
replicated server functions: functions that run on the server
muilticast: these run on both the client and server... call it on the server


1) On rep notify



	UPROPERTY(ReplicatedUsing = OnRep_Ammo)
	-This tells Unreal to replicate the Ammo variable from the server to the clients.
		But instead of just updating the value silently, Unreal will call a special function (OnRep_Ammo()) on the client when that value changes due to replication.

	OnRep_Ammo()
	-This is a replication notification function, often called an "OnRep".
		It is automatically called on clients when the server sends a new value for Ammo.
		You use this to trigger effects or update UI, etc.

	when ammo changes onrep_ammo will be called 
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
	int32 Ammo;

	
	UFUNCTION()
	void OnRep_Ammo();


	*how it works
	-Server changes Ammo value.
	-Unreal replicates the new Ammo value to clients.
	-On each client that receives the update:
	-Unreal assigns the new value to Ammo.
	-Then it automatically calls OnRep_Ammo().
	-In OnRep_Ammo(), you can add custom logic like playing a reload sound, updating HUD, triggering animations, etc.

	assigned as replicated prop as seen here 	DOREPLIFETIME(AWeapon, Ammo);




2) UFUNCTION(NetMulticast, Reliable) : called from the server run on the client


	Used for authoritative gameplay logic—the client requests something (e.g., reloading), but the server decides if it’s allowed and processes i
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);
	
	-This function is called on the server, but it will execute on all clients and the server
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	**how it works**
	-Player presses fire button (on the client).
	-Client calls a ServerFire() function (a Server RPC).
	-ServerFire() executes on the server, where the hit result is calculated.
	-The server then calls:


	//client calls fire and server executes fire... then calls muticast fire all the clients execute muticast fire
	void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
	{
		//this actor should fire on the server and the clients hence the multitcast... no matter you role fire that characters gun
		MulticastFire(TraceHitTarget);
	}
	
	//server_implementation was called form the client by 
	void UCombatComponent::Fire()
	{
		if (CanFire())
		{

			//telling the server i clicked fire and which point i was clicking at
			bCanFire = false;
			ServerFire(HitTarget);
			if (EquippedWeapon)
			{
				CrosshairShootingFactor = .75f;
			}
			StartFireTimer();
		}
	}


	serverfire is called from the client and executed on the server by
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);
	//then the server method signature is ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)



3) UFUNCTION(Server, Reliable) : called from the client run on the server

	-function called from the client but ran on the server
	UFUNCTION(Server, Reliable)
	void ServerReload()

	-Server in the macro ( Server: This means the function is called from the client, but it is executed on the server.)
	-Reliable in the macro (reliable: Ensures the function call is guaranteed to reach the server. Use this for critical actions 
		(like reloading, firing, etc.). If it were Unreliable, the call might be dropped under bad network conditions.


	-this is the effect and will run on the server. because we marked is as server, there is not ServerReload() only ServerReload_Implementation
	void UCombatComponent::ServerReload_Implementation()
	{
		if(Character == nullptr || EquippedWeapon == nullptr)
		return;


		CombatState = ECombatState::ECS_Reloading;
		HandleReload();//this only runs on the server... but then the combate state changes OnRep_CombatState will run on the client
	}


	so...Client calls ServerReload() The function runs on the server as ServerReload_Implementation, validating and triggering reload animation/state

4) replicated props

	void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);
		//replication does not run on the server so we need to handle a special case when the server overlaps
		//we are registering replicated variables here
		DOREPLIFETIME(UCombatComponent,EquippedWeapon);
		DOREPLIFETIME(UCombatComponent,bAiming);
		//replicating only on the client owner
		DOREPLIFETIME_CONDITION(UCombatComponent,CarriedAmmo, COND_OwnerOnly);
		DOREPLIFETIME(UCombatComponent,CombatState);
	}


	EquippedWeapon	All clients	Everyone sees the equipped weapon
	bAiming	All clients	Aiming state visible to all
	CarriedAmmo	Owning client only	Other players can’t see your ammo
	CombatState	All clients	Everyone sees your combat state
	

	5) Server vs NetMulticast
	UFUNCTION(Server, ...) vs	UFUNCTION(NetMulticast, ...)

	Called from =>	UFUNCTION(Server, ...) : Client (usually) or server |	UFUNCTION(NetMulticast, ...) : Server only
	Executes on	=> UFUNCTION(Server, ...) : Server only | UFUNCTION(NetMulticast, ...) : All clients and the server
	Use case => UFUNCTION(Server, ...) : Authoritative logic (e.g., firing, damage, reloading) | UFUNCTION(NetMulticast, ...) : Effects everyone must see (e.g., muzzle flash, animations)
	Delivery =>	UFUNCTION(Server, ...) : Runs once, on server | UFUNCTION(NetMulticast, ...) : Broadcasts and runs on all clients and server
	Reliable vs Unreliable => UFUNCTION(Server, ...) : Can be either | UFUNCTION(NetMulticast, ...) : Can be either (but Reliable is common for visible effects)






	***
	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade"));
	Use CreateDefaultSubobject<T>() when: You want the component to exist in every instance of the class. or You want the subobject to be part of the default object hierarchy
	Do NOT use CreateDefaultSubobject when: You want to create a transient or runtime-only object.

	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	This line is attaching a component (e.g., a grenade) to a specific socket on a skeletal mesh, like a character's hand or belt.

	so altogether these are doing... 
	1) Created as part of the character (or actor).
	2)Attached to a specific socket on the character's skeletal mesh (e.g., hand or belt).

*/