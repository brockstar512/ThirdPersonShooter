// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
	public:
		UFUNCTION(BlueprintCallable)
		void MenuSetup(int32 NumberOfPublicConnections = 4,FString TypeOfMatch = FString(TEXT("FreeForAll")),FString LobbyPath = FString(TEXT("/Game/ThirdPerson/Maps/Lobby")));

	protected:
		virtual bool Initialize() override;
		virtual void NativeDestruct() override;

		//
		//Callbacks for the custom delagtes on the MultiplayerSessionsSubsystem
		//
		UFUNCTION()
		void OnCreateSession(bool bWasSuccessful);//this is the type of delegate that can be bind to the delegate
		void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults,bool bWasSuccessful);
		void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
		UFUNCTION()
		void OnDestroySession(bool bWasSuccessful);
		UFUNCTION()
		void OnStartSession(bool bWasSuccessful);

	private:
		UPROPERTY(meta = (BindWidget))
		class UButton* HostButton;//name of the variable must have the same name as the widget in blueprints
		UPROPERTY(meta = (BindWidget))
		UButton* JoinButton;
	
		//because we are bding this function to an onclick event or delegate these have to be u functions
		UFUNCTION()
		void HostButtonClicked();
		UFUNCTION()
		void JoinButtonClicked();

		UFUNCTION()
		void MenuTearDown();
		//subsystem designed to handle all online session functionality
		class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

		int32 NumPublicConnections{4};
		FString MatchType{TEXT("FreeForAll")};
		FString PathToLobby{TEXT("")};

};
