#include "BlasterCharacter.h"
// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "ThirdPersonShooter/BlasterComponents/CombatComponent.h"
#include "ThirdPersonShooter/Weapon/Weapon.h"
#include "Kismet/KismetMathLibrary.h"
#include "ThirdPersonShooter/ThirdPersonShooter.h"
#include "BlasterAnimInstance.h"
#include "ThirdPersonShooter/PlayerController/BlasterPlayerController.h"
#include "ThirdPersonShooter/GameMode/BlasterGameMode.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "ThirdPersonShooter/PlayerState/BlasterPlayerState.h"
#include "ThirdPersonShooter/Weapon/WeaponTypes.h"
/*
* there are many characters/pawns in your game... these are all here because there will be one active player controller that you are interacting with. the logic that the other players need to do that you need to see but not effect with your own inputs will put placed in this class
*/
// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	//parenting the boom to the mesh so it wont change in size unlike if we parented it to the capsule
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom,USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation= false;

	bUseControllerRotationYaw  = false;
	GetCharacterMovement()-> bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	//when you hit this channel heres how you treat it
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera,ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	//if the camera collides with me ignore
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera,ECollisionResponse::ECR_Ignore);
	//we are going to block peoples raycasts
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	GetCharacterMovement()->RotationRate = FRotator(0.f,0.f,850.f);
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
}void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//replication does not run on the server so we need to hanlde a special case when the server overlaps
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	//register replicted health
	DOREPLIFETIME(ABlasterCharacter, Health);
	DOREPLIFETIME(ABlasterCharacter, bDisableGameplay);

}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if(Combat)
	{
		Combat->Character = this;//passing this ccharacter off to the combat class
	}
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateHUDHealth();
	if(HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");

		// UE_LOG(LogTemp, Warning, TEXT("string %s"), *SectionName.ToString());
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;

		switch(Combat->EquippedWeapon->GetWeaponType())
		{
			case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
			case EWeaponType::EWT_RocketLauncher:
				SectionName = FName("Rifle");
				break;
		}
		// UE_LOG(LogTemp, Warning, TEXT("string %s"), *SectionName.ToString());
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Black, FString::Printf(TEXT("Reloading")));
	}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ABlasterCharacter::Elim()
{
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->Dropped();
	}
	
	MulticastElim();
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ABlasterCharacter::ElimTimerFinished,
		ElimDelay
	);
}

void ABlasterCharacter::MulticastElim_Implementation()
{
	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0);
	}
	
	bElimmed = true;
	PlayElimMontage();
	//when you are eliminated attach disolve material
	if(DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0,DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}

	StartDissolve();

	// Disable character movement
	bDisableGameplay = true;
	// Disable character movement
	GetCharacterMovement()->DisableMovement();//stop movment
	GetCharacterMovement()->StopMovementImmediately();//stop turn rotation

	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Spawn elim bot
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotEffect,
			ElimBotSpawnPoint,
			GetActorRotation()
		);
	}
	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			ElimBotSound,
			GetActorLocation()
		);
	}
}

void ABlasterCharacter::ElimTimerFinished()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if (BlasterGameMode)
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}
}

void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABlasterCharacter::StartDissolve()
{
	//bind delegate track to the function
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);

	if(DissolveCurve && DissolveTimeline)
	{
		//interpolate the curve value over the time
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
			//UE_LOG(LogTemp, Warning, TEXT("Hit react montage start montage"));
			// 
			//plays hit montage only when player has a gun
			// 
			//bool isCombat = Combat == nullptr;
			//bool isWeapon = Combat->EquippedWeapon == nullptr;
	/*		UE_LOG(LogTemp, Error, TEXT("is combat null ? %s", isCombat ?TEXT("true") : TEXT("false"));
			UE_LOG(LogTemp, Error, TEXT("is equip weapon ? %s", isWeapoon ? TEXT("true") : TEXT("false"));*/

			//if (GEngine)
			//{
			//	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Black, FString::Printf(TEXT("isCombat: %s"), isCombat ? TEXT("true") : TEXT("false")));
			//}

			//if (GEngine)
			//{
			//	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Black, FString::Printf(TEXT("isWeapon: %s"), isCombat ? TEXT("true") : TEXT("false")));
			//}


	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

			//UE_LOG(LogTemp, Warning, TEXT("Hit react montage second stage"));


	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Hit react montage main stage"));

	
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
		//UE_LOG(LogTemp, Warning, TEXT("Hit react montage end montage"));

}

void ABlasterCharacter::ReceiveDamage(AActor * DamagedActor, float Damage, const UDamageType * DamageType, AController * InstigatorController, AActor * DamageCauser)
{
	//this will call on rep health... the replicated function will take care of playing the montage on the client
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	//variable replication is more effecient than rpc
	//this wil run on the server
	UpdateHUDHealth();
	PlayHitReactMontage();
	//UE_LOG(LogTemp, Warning, TEXT("char hit here is damage %f"), Damage);

	if(Health == 0.f)
	{

	

	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();

		if(BlasterGameMode)
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController,AttackerController);
		}
	}
}

void ABlasterCharacter::Jump()
{
	if (bDisableGameplay) return;
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ABlasterCharacter::Tick(float DeltaTime)
{

	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);
	HideCameraIfCharacterClose();
	PollInit();
}

void ABlasterCharacter::RotateInPlace(float DeltaTime)
{
	if (bDisableGameplay) 
	{ 
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return; 
	}

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		//if you are the server animated how you normally do
		AimOffset(DeltaTime);
	}
	else
	{
		//if you are not the server own player. 
		//handle the replicated movement 
		//get a reference to the last time we did a net update to replicate the movement
		TimeSinceLastMovementReplication += DeltaTime;
		//on rep changes only when we move, so we are creating a timesptamp to make sure we are updating the animattio n
		//even when we are not activly moving.
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		//do the pitch
		CalculateAO_Pitch();
	}
	//UE_LOG(LogTemp, Warning, TEXT("tick is running!"));

}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	//the server is controller the movement of the characters that don't have authority
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	//reset our value so we now the last time we updated on the net
	TimeSinceLastMovementReplication = 0.f;
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABlasterCharacter::Jump);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ABlasterCharacter::ReloadButtonPressed);

	//bind this macro from the input project settings mapping, with this function
	PlayerInputComponent->BindAxis("MoveForward",this,&ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight",this,&ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn",this,&ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp",this,&ABlasterCharacter::LookUp);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABlasterCharacter::FireButtonReleased);


}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{

	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
				// UE_LOG(LogTemp, Warning, TEXT("Turning right"));
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
		// UE_LOG(LogTemp, Warning, TEXT("Turning left"));
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;

		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
			// UE_LOG(LogTemp, Warning, TEXT("Not Turning"));
		}
	}
}

void ABlasterCharacter::HideCameraIfCharacterClose()
{
	if(!IsLocallyControlled())return;
	if((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if(Combat && Combat->EquippedWeapon && Combat-> EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if(Combat && Combat->EquippedWeapon && Combat-> EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}	
	}
}

// void ABlasterCharacter::MulticastHit_Implementation()
// {
// 	// UE_LOG(LogTemp, Warning, TEXT("Play Hit montage"));
// 	PlayHitReactMontage();
// }

//this will only run on thr server
void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{


	//i think we are forcing weapon off on the server then changes the overlapping variable so it fires on local client
	//then we are handling when the server is able to see the weapon...
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;

	//this handles the server prompt because on the server it will be locally owned
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	//is aiming is just referencing the aiming bool in our combat component
	return (Combat && Combat->bAiming);
}

AWeapon * ABlasterCharacter::GetEqippedWeapon()
{
	if(Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if(Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

ECombatState ABlasterCharacter::GetCombatState() const
{
	if(Combat== nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}

void ABlasterCharacter::MoveForward(float Value)
{
	if (bDisableGameplay) return;
	if(Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	if (bDisableGameplay) return;

		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
}

void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ABlasterCharacter::EquipButtonPressed()
{
	if (bDisableGameplay) return;

	if(Combat)
	{
		if(HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if (bDisableGameplay) return;

	if(bIsCrouched)
	{
		UnCrouch();
		if(GEngine)
		{
    		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("UNCrouched!"));	
		}
	}
	else
	{

		Crouch();
		if(GEngine)
		{
    		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Crouched!"));	
		}
	}
}

void ABlasterCharacter::AimButtonPressed()
{
	if (bDisableGameplay) return;

	if(!IsWeaponEquipped()) return;
	//we are setting the combat component aiming bool in our blaster script
	if(Combat)
	{
		Combat->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonReleased()
{
	if (bDisableGameplay) return;

	if(Combat)
	{
		Combat->SetAiming(false);
	}
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) // standing still, not jumping
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir) // running, or jumping
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void ABlasterCharacter::SimProxiesTurn()
{
	//simulate the turn 
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	//if we are running we start sliding.. when we are running we are preventing us from going into the turning state 
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	//get the last rotatioj
	//then update the rotation
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	//UE_LOG(LogTemp, Warning, TEXT("ProxyYaw: %f"), ProxyYaw);
	//determine the turn based of the delta
	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

}

float ABlasterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ABlasterCharacter::OnRep_Health()
{
	//it's going to get subtracted on the server then when that changes this function runs on the client
	//UE_LOG(LogTemp,Display,TEXT("HEALTH Rep:: %f"),Health);

	UpdateHUDHealth();
	PlayHitReactMontage();
}

void ABlasterCharacter::CalculateAO_Pitch()
{
		AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	if (bDisableGameplay) return;

	// if(GEngine)
	// {
    // 	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("fire button released"));	
	// }
	if(Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void ABlasterCharacter::FireButtonPressed()
{	
	if (bDisableGameplay) return;

	// if(GEngine)
	// {
    // 	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("fire button pressed"));	
	// }
	if(Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

//this runs on clients
void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	// if(GEngine)
	// {
    // 	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("on repo overlap fired because OverlappingWeapon changed"));	
	// }

	//when the variable changes to null as we are leaving this condition will fail
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	//but we will have a cached reference to what the last weapon was and we can difrectly call that weapon to hide the widget
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
	
}
//we need to add _implementation so the engine knows this is the implementation... the low level stuff is going to be taken
//are of by the engine
void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
		if(Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	
	if(BlasterPlayerController)
	{
		//UE_LOG(LogTemp,Display,TEXT("HEALTH:: %f"),Health);//this playes onces
		BlasterPlayerController->SetHUDHealth(Health,MaxHealth);
	}
}

void ABlasterCharacter::PollInit()
{
	if(BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if(BlasterPlayerState)
		{
			BlasterPlayerState->AddToScore(0.f);
			BlasterPlayerState->AddToDefeats(0);

		}
	}
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}

	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;
	if (Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}
}

void ABlasterCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay) return;

	if(Combat)
	{
		Combat->Reload();
	}
}