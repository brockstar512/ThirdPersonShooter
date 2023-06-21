// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

/**
 * 
 */
UCLASS()
class THIRDPERSONSHOOTER_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()
	
	public:
		UPROPERTY(meta=(BindWidget))//any change we make to this text block will be affected by the widget
		class UTextBlock* DisplayText;

		void SetDisplayText(FString TextToDisplay);
		
		UFUNCTION(BlueprintCallable)
		void ShowPlayerNetRole(APawn* InPawn);

	protected:
	// 	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;
	
		virtual void NativeDestruct() override;


};


