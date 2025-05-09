#pragma once

#define TRACE_LENGTH 80000.f


/*This macro tells Unreal Engine to expose the enum to Blueprints, meaning you can use EWeaponType in the Blueprint visual scripting system.
: uint8 ensures it uses only 1 byte of memory — efficient for things like networking or storage.*/
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    /*
    EWT_AssaultRifle: The internal C++ name.
    UMETA(DisplayName = "Assault Rifle"): The name shown in Blueprints/editor.
    */
    EWT_AssaultRifle UMETA(DisplayName ="Assault Rifle"),
    EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
    EWT_Pistol UMETA(DisplayName = "Pistol"),
    EWT_SubMachineGun UMETA(DisplayName = "SubMachineGun"),
    EWT_Shotgun UMETA(DisplayName = "Shotgun"),
    EWT_SniperRifle UMETA(DisplayName = "Sniper Rifle"),
    EWT_GrenageLauncher UMETA(DisplayName = "Grenade Launcher"),


    EWT_MAX UMETA(DisplayName = "DefaultMAX")
};