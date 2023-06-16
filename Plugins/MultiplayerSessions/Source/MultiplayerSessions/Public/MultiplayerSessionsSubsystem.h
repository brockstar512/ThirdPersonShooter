// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionsSubsystem.generated.h"
/**
 * 
 */

//
//Declarring our own custmi delegates for the menu class to bind callbacks to
//
//type of delegate								name of delegate      -     input type - name for input paramter
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool,bWasSuccessful);
// the delegate can be serialized and used in a blueprint/this meancs multiple functions can bind to it./number of params 
//because its dynamic multicast delgate any functions we bind to it, it needs to be a ufunction

//this will not be a dynamic delagate because blue prints dont support non Uclass or UStructs which onlisne session seach result is not
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResults,bool bWasSuccessful);
																		//passing in a reference so we dont have to copy array

DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool,bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool,bWasSuccessful);


UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
	public:
		UMultiplayerSessionsSubsystem();

		//To Handle session functionality. The menu class will call this

		void CreateSession(int32 NumPublicConnections, FString MatchType);
		void FindSessions(int32 MaxSearchResults);
		void JoinSession(const FOnlineSessionSearchResult& SessionResult);
		void DestroySession();
		void StartSession();

		//
		//Our own custom delegates for the menu class to bind callbacks to
		//
		FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
		FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionsComplete;
		FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;
		FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
		FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;

	protected:
		//internal callbacks for the delegates we'll add to the online session interface delegate list
		//these sone need to be called outside of this class

		void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
		void OnFindSessionsComplete(bool bWasSuccessful);
		void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
		void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
		void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);




	private:
		IOnlineSessionPtr SessionInterface;
		TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
		TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

		//To add to the online session interface delegate list
		//we'll bind our  Multiplayer subsystem internal callbacks to these.
		//the handles are there so we can remove them from the list later when we do not need them
		FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
		FDelegateHandle CreateSessionCompleteDelegateHandle;
		FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
		FDelegateHandle FindSessionsCompleteDelegateHandle;
		FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
		FDelegateHandle JoinSessionCompleteDelegateHandle;
		FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
		FDelegateHandle DestroySessionCompleteDelegateHandle;
		FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
		FDelegateHandle StartSessionCompleteDelegateHandle;
		
		bool bCreateSessionOnDestroy{false};
		int32 LastNumPublicConnections;
		FString LastMatchType;
};
