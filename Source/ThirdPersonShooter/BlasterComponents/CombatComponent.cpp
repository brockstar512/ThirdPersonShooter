// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "ThirdPersonShooter/Weapon/Weapon.h"
#include "ThirdPersonShooter/Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"



// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	//we are going to leave this here so we have to wait for the server to change this variable and have a lag in the animation
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);

}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
		bAiming = bIsAiming;
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
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
}

