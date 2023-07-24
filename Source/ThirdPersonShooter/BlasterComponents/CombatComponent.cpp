// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "ThirdPersonShooter/Weapon/Weapon.h"
#include "ThirdPersonShooter/Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "ThirdPersonShooter/PlayerController/BlasterPlayerController.h"
#include "ThirdPersonShooter/HUD/BlasterHUD.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 400.f;

	// ...
}


// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}
	
}

// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	//UE_LOG(LogTemp, Warning, TEXT("Hello"));
	// ...
	// FHitResult HitResult;
	// TraceUnderCrosshairs(HitResult);
	SetHUDCrosshairs(DeltaTime);
}


void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//replication does not run on the server so we need to hanlde a special case when the server overlaps
	//we are registering replicated variables here
	DOREPLIFETIME(UCombatComponent,EquippedWeapon);
	DOREPLIFETIME(UCombatComponent,bAiming);

}

void UCombatComponent::EquipWeapon(AWeapon * WeaponToEquip)
{
	if(Character == nullptr|| WeaponToEquip == nullptr)
	{
		return;
	}

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));

	if(HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon,Character->GetMesh());
	}

	EquippedWeapon->SetOwner(Character);
	// EquippedWeapon->ShowPickupWidget(false);
	// EquippedWeapon->GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	//we are going to leave this here so we have to wait for the server to change this variable and have a lag in the animation
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}

}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed)
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		//telling the server i clicked fire and which point i was clicking at
		ServerFire(HitResult.ImpactPoint);
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}


void UCombatComponent::OnRep_EquippedWeapon()
{
	if(EquippedWeapon && Character)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	//this actore should fire on the server and the clients hence the multitcast... no matter you role fire that characters gun
	MulticastFire(TraceHitTarget);
}

//this is the server calling fire on all clients and server
void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if(EquippedWeapon == nullptr) return;
	//checking if this role has a gun and character then fireing
	if (Character)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}

}

void UCombatComponent::TraceUnderCrosshairs(FHitResult &TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	UE_LOG(LogTemp, Warning, TEXT("got to ray function!"));

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		//UE_LOG(LogTemp, Warning, TEXT("sending out ray!"));

		FVector Start = CrosshairWorldPosition;

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);


			// DrawDebugSphere(
			// 	GetWorld(),
			// 	TraceHitResult.ImpactPoint,
			// 	12.f,
			// 	12,
			// 	FColor::Red
			// );
		
	}

}


void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	// UE_LOG(LogTemp, Log, TEXT("Bool 1 value is: %s"), Character->Controller == nullptr ? "true" : "false" );
	// UE_LOG(LogTemp, Log, TEXT("Bool 2 value is: %s"), Controller == nullptr ? "true" : "false" );
	// if (GEngine)
	// {
	// 	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Black, FString::Printf(TEXT("Bool: %s"), Character->Controller == nullptr ? TEXT("true") : TEXT("false")));
	// }
	if (Character == nullptr || Character->Controller == nullptr) return;
	UE_LOG(LogTemp, Log, TEXT("casting controller"));
	// UE_LOG(LogTemp, Warning, TEXT("Hello"));
	//assign this vairable controller to the controller in character
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;

	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
		UE_LOG(LogTemp, Log, TEXT("controller exists"));

		if (HUD)
		{
			FHUDPackage HUDPackage;
			if (EquippedWeapon)
			{
						UE_LOG(LogTemp, Log, TEXT("hud exists"));

				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				UE_LOG(LogTemp, Warning, TEXT("Got textures!"));

			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
			}
			

			HUD->SetHUDPackage(HUDPackage);
		}
	}else
	{
	UE_LOG(LogTemp, Log, TEXT("no controller"));

	}
}


