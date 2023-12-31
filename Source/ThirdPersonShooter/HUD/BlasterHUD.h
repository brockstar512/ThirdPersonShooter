// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:
	class UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;
	float CrosshairSpread;
	FLinearColor CrosshairsColor;
};
/**
 * 
 */
UCLASS()
class THIRDPERSONSHOOTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override;
	UPROPERTY(EditAnywhere, Category = "Player Stats")//this is going to be a generic type as long as it inheretis from userwudget
	TSubclassOf<class UUserWidget> CharacterOverlayClass;//this will let us create the widget in the blueprint
	class UCharacterOverlay* CharacterOverlay;
private:
	FHUDPackage HUDPackage;
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread,FLinearColor CrosshairsColor);
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;
	
protected:
	virtual void BeginPlay() override;
	void AddCharacterOverlay();
public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package){HUDPackage = Package;}

};
