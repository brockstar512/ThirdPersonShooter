// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

void UMenu::MenuSetup(int32 NumberOfPublicConnections,FString TypeOfMatch,FString LobbyPath)
{
    PathToLobby = FString::Printf(TEXT("%s?listen"),*LobbyPath);
    NumPublicConnections = NumberOfPublicConnections;
    MatchType = TypeOfMatch;
    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    bIsFocusable = true;

    UWorld* World = GetWorld();
    if(World)
    {
        APlayerController* PlayerController = World->GetFirstPlayerController();
        if(PlayerController)
        {
            //restricts input mode to ui
            FInputModeUIOnly InputModeData;
            InputModeData.SetWidgetToFocus(TakeWidget());
            InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PlayerController->SetInputMode(InputModeData);
            PlayerController->SetShowMouseCursor(true);


        }
    }
    UGameInstance* GameInstance = GetGameInstance();
    if(GameInstance)
    {
        //get the session subsystem class
       MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
    }

    if(MultiplayerSessionsSubsystem)
    {
        //binding the delgate here
        MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this,&ThisClass::OnCreateSession);
        MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this,&ThisClass::OnFindSessions);
        MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this,&ThisClass::OnJoinSession);
        MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this,&ThisClass::OnDestroySession);
        MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this,&ThisClass::OnStartSession);


    }

    
   
}

bool UMenu::Initialize()
{
    if(!Super::Initialize())
    {
        return false;
    }
    //bind buttons
    if(HostButton)
    {
        HostButton->OnClicked.AddDynamic(this, &UMenu::HostButtonClicked);
    }
    if(JoinButton)
    {
        JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
    }

    return true;
}

void UMenu::HostButtonClicked()
{
        HostButton->SetIsEnabled(false);

   
     if(MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections,MatchType);

    }
}

void UMenu::JoinButtonClicked()
{
    JoinButton->SetIsEnabled(false);

    if(MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->FindSessions(10000);
    }

}

void UMenu::NativeDestruct()
{
	MenuTearDown();
 
	Super::NativeDestruct();
}

void UMenu::MenuTearDown()
{
    RemoveFromParent();
    UWorld* World = GetWorld();

    if(World)
    {
        APlayerController* PlayerController = World->GetFirstPlayerController();
        if(PlayerController)
        {
            //restricts input mode to game 
            FInputModeGameOnly InputModeData;
            PlayerController->SetInputMode(InputModeData);
            PlayerController->SetShowMouseCursor(false);
        }
    }

}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
    if(bWasSuccessful)
    {
        if(GEngine){
            GEngine->AddOnScreenDebugMessage(
                -1,
                15.f,
                FColor::Yellow,
                FString(TEXT("Session Created Successfuly"))
            );
        }

        UWorld* World = GetWorld();
        if(World)
        {
            //if we are creating the session and are traveling then we are the listen server
            //we are going to add the listen option so we can wait for others
            //World->ServerTravel("/Game/ThirdPersonCPP/Maps/Lobby?listen");
            World->ServerTravel(PathToLobby);

        }
    }
    else
    {
        if(GEngine){
            GEngine->AddOnScreenDebugMessage(
                -1,
                15.f,
                FColor::Red,
                FString(TEXT("Session Creation Failed"))
            );
        }
            HostButton->SetIsEnabled(true);

    }


}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults,bool bWasSuccessful)
{
    if(MultiplayerSessionsSubsystem == nullptr)
    {
        return;
    }

    for(auto Result : SessionResults)
    {
        FString SettingsValue;
		//getting the value of the key matchtype that was passed in
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
        if(SettingsValue == MatchType)
        {
            MultiplayerSessionsSubsystem->JoinSession(Result);
            return;
        }
    }
    //failed or cant find a session
    if(!bWasSuccessful || SessionResults.Num( )== 0)
    {
        JoinButton->SetIsEnabled(true);
    }
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
    //get the correct address and travel to that player
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();

    if(Subsystem)
    {
        IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
        
        if(SessionInterface.IsValid())
        {
            FString Address;
            SessionInterface->GetResolvedConnectString(NAME_GameSession,Address);

		    APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();

		    if(PlayerController)
		    {
			    //we are getting the address we need to trael to 
			    PlayerController->ClientTravel(Address,ETravelType::TRAVEL_Absolute);
		    }
        }
    }
    //if you were not able to join the session
    if(Result != EOnJoinSessionCompleteResult::Success)
    {
            JoinButton->SetIsEnabled(true);
    }

}

void UMenu::OnDestroySession(bool bWasSuccessful)
{

}

void UMenu::OnStartSession(bool bWasSuccessful)
{

}
