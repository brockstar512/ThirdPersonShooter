// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():
CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();

    if(Subsystem)
    {
        SessionInterface = Subsystem->GetSessionInterface();
    }
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if(SessionInterface)
    {
        //remove the delagate that was binded to this function in the first place... then make the broadcast
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
    }
    
    MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
    //clear delegate from the list
    if(SessionInterface)
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
    }
    if(LastSessionSearch->SearchResults.Num()<=0)
    {
        MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(),false);
        return;
    }

    //broadcast to the menu
    MultiplayerOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults,bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if(SessionInterface)
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
    }

    MultiplayerOnJoinSessionComplete.Broadcast(Result);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    if(SessionInterface)
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
    }

    if(bWasSuccessful && bCreateSessionOnDestroy)
    {
        bCreateSessionOnDestroy = false;
        CreateSession(LastNumPublicConnections,LastMatchType);
    }
    MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{

}

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
    if(!SessionInterface.IsValid())
    {
        return;
    }
    auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
    if(ExistingSession != nullptr)
    {
        bCreateSessionOnDestroy = true;
        LastNumPublicConnections = NumPublicConnections;
        LastMatchType = MatchType;
        // SessionInterface->DestroySession(NAME_GameSession);
        DestroySession();
    }
        //store the delegate in a FDelegateHandle so we can later remove it from the delegate list
        CreateSessionCompleteDelegateHandle = SessionInterface -> AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

        LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
        //checking if ths is the null subsystem
        LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName()=="NULL" ? true : false;

        LastSessionSettings->NumPublicConnections = NumPublicConnections;
        LastSessionSettings->bAllowJoinInProgress = true;
        LastSessionSettings->bAllowJoinViaPresence = true;
        LastSessionSettings->bShouldAdvertise = true;
        LastSessionSettings->bUsesPresence = true;
        LastSessionSettings->bUseLobbiesIfAvailable = true;
        LastSessionSettings->Set(FName("MatchType"),MatchType,EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);		
        const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
        LastSessionSettings-> BuildUniqueId = 1;//keeps different builds from seeing echother. so we can see others and not just the first one we foudn

        //we are checking if the create session failed
    if(!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(),NAME_GameSession,*LastSessionSettings))
    {
        //remove the delegate from the list if the session creation failed
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

        //broadcast our own custom delagte
        MultiplayerOnCreateSessionComplete.Broadcast(false);
    }
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
    if(!SessionInterface.IsValid())
    {
        return;
    }

    FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

    //MakeShareable utility function. Wrap object pointers with MakeShareable to allow them 
    //to be implicitly converted to shared pointers! This is useful in assignment operations, 
    //or when returning a shared pointer from a function.
    LastSessionSearch = MakeShareable(new FOnlineSessionSearch());//constrcuting a FOnlineSessionSearch shared pointer
    //implicit means some details are not stated but can be determined without 
    //being stated because enough information is available

    LastSessionSearch->MaxSearchResults = MaxSearchResults;
    LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName()=="NULL" ? true : false;
	
	//make sure the session we are querrying uses presence...
	//because we are using presence we have to specify that
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
    
    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    //if this returns false... clear delegate and broadcast custom delegate t0 menu
	if(!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(),LastSessionSearch.ToSharedRef()))
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

        MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(),false);
    }
    //if find sessions is successful the delegate will already call find sessions complete
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
    if(!SessionInterface.IsValid())
    {
        MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
        return;
    }
    JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();

    if(!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(),NAME_GameSession,SessionResult))
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

        MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
    }
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
    if(!SessionInterface.IsValid())
    {
        MultiplayerOnDestroySessionComplete.Broadcast(false);
        return;
    }
    DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
    if(SessionInterface->DestroySession(NAME_GameSession))
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
        MultiplayerOnDestroySessionComplete.Broadcast(false);
    }
}

void UMultiplayerSessionsSubsystem::StartSession()
{

}