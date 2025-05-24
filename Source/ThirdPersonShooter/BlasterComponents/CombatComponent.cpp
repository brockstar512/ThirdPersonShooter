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
// #include "ThirdPersonShooter/HUD/BlasterHUD.h"//we do not need this here now because we included it now in the header
#include "ThirdPersonShooter/Character/BlasterAnimInstance.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "ThirdPersonShooter/Weapon/Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"

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

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		if(Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}

		if (Character->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}

}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if(EquippedWeapon == nullptr) return;

	if(bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(),DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	if(Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return;
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
	ReloadEmptyWeapon();
}

bool UCombatComponent::CanFire()
{
	if(EquippedWeapon == nullptr) return false;

	if (!EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun) return true;

	return !EquippedWeapon ->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;

}

void UCombatComponent::OnRep_CarriedAmmo()
{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDCarriedAmmo(CarriedAmmo);
		}
		bool bJumpToShotgunEnd =
			CombatState == ECombatState::ECS_Reloading &&
			EquippedWeapon != nullptr &&
			EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun &&
			CarriedAmmo == 0;

		if (bJumpToShotgunEnd)
		{
			JumpToShotgunEnd();
		}
	
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle,StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubMachineGun, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperRifleAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenageLauncher, StartingGrenadeLauncherAmmo);

	if (GEngine)
	{
		FString Message = FString::Printf(TEXT(
			"Initialized Ammo:\n"
			"AssaultRifle: %d\n"
			"RocketLauncher: %d\n"
			"Pistol: %d\n"
			"SubMachineGun: %d\n"
			"Shotgun: %d\n"
			"SniperRifle: %d\n"
			"GrenadeLauncher: %d"),
			StartingARAmmo,
			StartingRocketAmmo,
			StartingPistolAmmo,
			StartingSMGAmmo,
			StartingShotgunAmmo,
			StartingSniperRifleAmmo,
			StartingGrenadeLauncherAmmo
		);

		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, Message);
	}
}

void UCombatComponent::UpdateHudGrenades()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDGrenades(Grenades);

	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	//UE_LOG(LogTemp, Warning, TEXT("Hello"));
	// ...
	// FHitResult HitResult;
	// TraceUnderCrosshairs(HitResult);

	
	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;


		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::ShotGunShellReload()
{
	if (Character && Character->HasAuthority()) {
		UpdateShotgunAmmoValues();
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//replication does not run on the server so we need to handle a special case when the server overlaps
	//we are registering replicated variables here
	DOREPLIFETIME(UCombatComponent,EquippedWeapon);
	DOREPLIFETIME(UCombatComponent,bAiming);
	DOREPLIFETIME(UCombatComponent,CombatState);
	DOREPLIFETIME(UCombatComponent, Grenades);


	//replicating only on the client owner
	DOREPLIFETIME_CONDITION(UCombatComponent,CarriedAmmo, COND_OwnerOnly);


}

void UCombatComponent::DroppedEquippedWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
}

void UCombatComponent::AttachActorToRightHand(AActor* AActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || AActorToAttach == nullptr)
	{
		return;
	}
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));

	if (HandSocket)
	{
		HandSocket->AttachActor(AActorToAttach, Character->GetMesh());
	}

}

void UCombatComponent::AttachActorToLeftHand(AActor* AActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || AActorToAttach == nullptr || EquippedWeapon == nullptr)
	{
		return;
	}
	bool bUseSmallGunSocket = EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol ||
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SubMachineGun;
	FName socketName = bUseSmallGunSocket ? FName("SmallGunSocket") : FName("LeftHandSocket");
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(socketName);

	if (HandSocket)
	{
		HandSocket->AttachActor(AActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) return;

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;

	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::PlayEquippedWeaponSound()
{
	if ( Character && EquippedWeapon && EquippedWeapon->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			EquippedWeapon->EquipSound,
			Character->GetActorLocation()
		);
	}
}

void UCombatComponent::ReloadEmptyWeapon()
{
	if (EquippedWeapon && EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::EquipWeapon(AWeapon * WeaponToEquip)
{
	if(Character == nullptr|| WeaponToEquip == nullptr)
	{
		return;
	}
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	
	DroppedEquippedWeapon();
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();
	PlayEquippedWeaponSound();
	ReloadEmptyWeapon();
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::Reload()
{
	//when we want to reload tell the server to reload
	if (CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && EquippedWeapon && !EquippedWeapon->IsFull())
	{
		ServerReload();
	}
}

void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}

void UCombatComponent::FinishReloading()
{
	if(Character == nullptr) return;

	if(Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
		// UE_LOG(LogTemp, Warning, TEXT("updated ammo values!"));

	}
	if(bFireButtonPressed)
	{
		Fire();

	}

}

void UCombatComponent::HandleReload()
{
    Character->PlayReloadMontage();
}

int32 UCombatComponent::AmountToReload()
{
	if(EquippedWeapon == nullptr)
	{
		return 0;
	}
	//how empty is our mag
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();

	//how much room in mag we have to pull from
	if (GEngine)
	{
		FString Message = FString::Printf(TEXT("Room in mag: %d"), RoomInMag);
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, Message);
	}
	//are we reading the correct weapon
	if (GEngine)
	{
		bool bHasAmmo = CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType());
		FString Message = FString::Printf(TEXT("Does ammo exist: %s"), bHasAmmo ? TEXT("true") : TEXT("false"));
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, Message);
	}



	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		//how much do I have in my reserve
		int32 AmountCarried  = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		//get what I can... if I dont have enough ammo to fill the emptiness take that if I have enough in the reserve take enough to just fill the emptiness
		int32 Least = FMath::Min(RoomInMag,AmountCarried);
		return FMath::Clamp(RoomInMag,0,Least);
	}
    return 0;
}

void UCombatComponent::ServerReload_Implementation()
{
	if(Character == nullptr || EquippedWeapon == nullptr)
	return;
	

	CombatState = ECombatState::ECS_Reloading;
	HandleReload();//this only runs on the server... but then the combate state changes OnRep_CombatState will run on the client
}

void UCombatComponent::OnRep_CombatState()
{
	switch(CombatState)
	{
		case ECombatState::ECS_Reloading:
		HandleReload();///this runs on the client
		break;
		case ECombatState::ECS_Unoccupied:
		if(bFireButtonPressed)
		{
			Fire();
		}	
		break;
		case ECombatState::ECS_ThrowingGrenade:
			if (Character && !Character->IsLocallyControlled())
			{
				Character->PlayThrowGrenadeMontage();
				AttachActorToLeftHand(EquippedWeapon);
				ShowAttachedGrenade(true);

			}
			break;

	}
}

void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || Character->Controller == nullptr) return;

	int32 ReloadAmount = AmountToReload();

	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	EquippedWeapon->AddAmmo(-ReloadAmount);

}

void UCombatComponent::UpdateShotgunAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(-1);
	bCanFire = true;
	if (EquippedWeapon->IsFull() || CarriedAmmo == 0)
	{
		JumpToShotgunEnd();
	}

}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (Character && Character->GetAttachedGrenade())
	{
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade);
	}
}

void UCombatComponent::OnRep_Grenades()
{
	UpdateHudGrenades();
}

void UCombatComponent::JumpToShotgunEnd()
{
	// Jump to ShotgunEnd section
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (AnimInstance && Character->GetReloadMontage())
	{
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	//we are going to leave this here so we have to wait for the server to change this variable and have a lag in the animation
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}

	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
	}

}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::LaunchGrenade()
{
	ShowAttachedGrenade(false);
	//this function is called on all instances from the animation blueprint... so we only want to tell the server if we are locally controlled
	if (Character && Character->IsLocallyControlled())
	{
		ServerLaunchGrenade(HitTarget);
	}
}

//server rpc's are called on the server
void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if (Character && GrenadeClass && Character->GetAttachedGrenade())
	{
		const FVector StartingLocation = Character->GetAttachedGrenade()->GetComponentLocation();
		FVector ToTarget = Target - StartingLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		UWorld* World = GetWorld();
		if (World)
		{
			//if i remove projectile arch remove the cahcing of the AProjectile* SpawnedProjectile
			AProjectile* SpawnedProjectile = World->SpawnActor<AProjectile>(
				GrenadeClass,
				StartingLocation,
				ToTarget.Rotation(),
				SpawnParams
			);

			//i added  this might not be correct
			//i did this... it might not work properly this ->
			if (SpawnedProjectile)
			{
				// Get the projectile movement component
				UProjectileMovementComponent* ProjectileMovement = SpawnedProjectile->GetProjectileMovementComponent();
				if (ProjectileMovement)
				{
					// Modify the velocity's Z component

					// Option 2: Add an upward boost (uncomment if needed)
					ProjectileMovement->Velocity += FVector(0.f, 0.f, SpawnedProjectile->GetArchVelocity());
				}
			}
			// <- to this
		}
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

void UCombatComponent::ThrowGrenade()
{
	if (Grenades == 0) return;
	if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr) return;
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	//if a listen server client (host) is here it will not be called on the server.
	//the other clients will tell the server to throw the grenades
	if (Character && !Character->HasAuthority())
	{
		//the clients will tell the server when to throw the grenades to update the game of other remote clients
		//then the listen server who is the client will be able to throw the grenades through the on rep update combat state function
		ServerThrowGrenade();
	}
	if (Character && Character->HasAuthority())
	{
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
		UpdateHudGrenades();
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if (Grenades == 0) return;

	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);

	}
	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
	UpdateHudGrenades();
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if(EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		PlayEquippedWeaponSound();
	}
}

//client calls fire -> runs on server -> executes on all the client that this this character is firing
void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	//this actore should fire on the server and the clients hence the multitcast... no matter you role fire that characters gun
	MulticastFire(TraceHitTarget);
}

//this is the server calling fire on all clients and server
void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if(EquippedWeapon == nullptr) return;

	if (Character && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun)
	{
		//only on the shotgun can weapon fire while relaoding
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
		CombatState = ECombatState::ECS_Unoccupied;
		return;
	}

	//checking if this role has a gun and character then if it can fire in its state
	if (Character && CombatState == ECombatState::ECS_Unoccupied)
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

	//get the center of the viewport
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
		if(Character)
		{
			//extending ray so it starts ahead of character not the camera
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			//if it a little offest
			Start+= CrosshairWorldDirection * (DistanceToCharacter + 50.f);
			// 	DrawDebugSphere(
			// 	GetWorld(),
			// 	Start,
			// 	12.f,
			// 	12,
			// 	FColor::Red,
			// 	false
			// );
		}

		//end point of our range
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);

		//check if there was no hit
		/*
			true = The trace hit something that is set to "Block" for the given collision channel.
			false = The trace did not hit anything that blocks the trace channel.
		*/
		if (!TraceHitResult.bBlockingHit)
		{
			//we need the end point somewhere, so draw it to that max distance
			TraceHitResult.ImpactPoint = End;
		}
		//turn the cross hairs red if it was a player
										//get  the actor and check if it implements the interface
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White;
		}
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
	//UE_LOG(LogTemp, Log, TEXT("Top of hud"));
	// UE_LOG(LogTemp, Warning, TEXT("Hello"));
	//assign this vairable controller to the controller in character
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;

	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
		// UE_LOG(LogTemp, Log, TEXT("controller exists"));

		if (HUD)
		{
			//UE_LOG(LogTemp, Log, TEXT("Got HUD"));

			
			if (EquippedWeapon)
			{
				//UE_LOG(LogTemp, Log, TEXT("Got crosshairs"));

				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
			}
			// Calculate crosshair spread

			// [0, 600] -> [0, 1]
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			if (bAiming)
			{

				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			//issue... seems like its immediatly turning the number to 0
			//always interpolate factor down to 0
			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 10.f);

			HUDPackage.CrosshairSpread = 
				0.5f + 
				CrosshairVelocityFactor + 
				CrosshairInAirFactor -
				CrosshairAimFactor +
				CrosshairShootingFactor;

			HUD->SetHUDPackage(HUDPackage);
		}
		else
		{
			// UE_LOG(LogTemp, Log, TEXT("Cound not get hud"));

		}
	}
}


void UCombatComponent::Fire()
{
	if (CanFire())
	{
		//if (GEngine)
		//{
		//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("combat component sending to server"));
		//}
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
