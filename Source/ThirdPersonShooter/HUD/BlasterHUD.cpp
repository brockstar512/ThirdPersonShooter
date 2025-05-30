// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"
#include "GameFramework/PlayerController.h"
#include "CharacterOverlay.h"
#include "Announcement.h"
void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();

	
	
}
void ABlasterHUD::AddAnnouncement()
{
	APlayerController* PlayerController = GetOwningPlayerController();

						//does our player exist and the subclass widget model
	if (PlayerController && AnnouncementClass)
	{
		//cache the class after we create the widget and parent it to the owner and the model
		Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
		//add it to our screen
		Announcement->AddToViewport();

	}
}
void ABlasterHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController =GetOwningPlayerController();

	if(PlayerController && CharacterOverlayClass)
	{
		//we have a varaibel for a widget
					//we are assigning that variable
							//we are creating that variable using the UCharacterOverlay model
													//we are passing in the player controller to which character
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController,CharacterOverlayClass);
		//add it to our screen
		CharacterOverlay->AddToViewport();
	}
}

void ABlasterHUD::DrawHUD()
{
    Super::DrawHUD();
    //UE_LOG(LogTemp, Warning, TEXT("Drawing!"));

   FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		if (HUDPackage.CrosshairsCenter)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread,HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread,HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsRight)
		{
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread,HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsTop)
		{
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread,HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsBottom)
		{
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread,HUDPackage.CrosshairsColor);
		}
	}
} 

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread,FLinearColor CrosshairsColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y
	);

	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		CrosshairsColor
	);
}


