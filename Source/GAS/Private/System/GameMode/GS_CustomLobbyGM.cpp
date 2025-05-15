#include "System/GameMode/GS_CustomLobbyGM.h"
#include "System/PlayerController/GS_CustomLobbyPC.h"
#include "System/GS_PlayerState.h"
#include "System/GameState/GS_CustomLobbyGS.h"
#include "Kismet/GameplayStatics.h"

AGS_CustomLobbyGM::AGS_CustomLobbyGM()
{
	PlayerStateClass = AGS_PlayerState::StaticClass();
	PlayerControllerClass = AGS_CustomLobbyPC::StaticClass();
	GameStateClass = AGS_CustomLobbyGS::StaticClass();
}

void AGS_CustomLobbyGM::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AGS_PlayerState* PS = NewPlayer->GetPlayerState<AGS_PlayerState>();
	if (PS)
	{
		PlayerReadyStates.Add(PS, false);
		PS->InitializeDefaults();
		UE_LOG(LogTemp, Log, TEXT("LobbyGM: Player %s logged in. Total players: %d"), *PS->GetPlayerName(), PlayerReadyStates.Num());
	}
}

void AGS_CustomLobbyGM::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	AGS_PlayerState* PS = Cast<AGS_PlayerState>(Exiting->PlayerState);
	if (PS)
	{
		PlayerReadyStates.Remove(PS);
		UE_LOG(LogTemp, Log, TEXT("LobbyGM: Player %s logged out. Total players: %d"), *PS->GetPlayerName(), PlayerReadyStates.Num());
		CheckAllPlayersReady();
	}
}

void AGS_CustomLobbyGM::UpdatePlayerReadyStatus(APlayerState* Player, bool bIsReady)
{
	if (Player)
	{
		bool* FoundStatus = PlayerReadyStates.Find(Player);
		if (FoundStatus)
		{
			*FoundStatus = bIsReady;
			UE_LOG(LogTemp, Log, TEXT("LobbyGM: Player %s is now %s"), *Player->GetPlayerName(), bIsReady ? TEXT("Ready") : TEXT("Not Ready"));
			CheckAllPlayersReady();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("LobbyGM: Player %s not found in PlayerReadyStates"), *Player->GetPlayerName());
		}
	}
}

void AGS_CustomLobbyGM::CheckAllPlayersReady() //����� 1�� �ƴϸ� ���� �� �Ǵ� ���� �߰��ؾ���
{
    AGS_CustomLobbyGS* LGS = GetGameState<AGS_CustomLobbyGS>();
    if (!LGS)
    {
        UE_LOG(LogTemp, Error, TEXT("LobbyGM: AGS_CustomLobbyGS is null in CheckAllPlayersReady."));
        return;
    }

    const int32 CurrentPlayerCount = LGS->PlayerArray.Num();
    UE_LOG(LogTemp, Verbose, TEXT("LobbyGM: Checking ready status for %d players in GameState. Min players: %d. Players in local map: %d"),
        CurrentPlayerCount, MinPlayersToStart, PlayerReadyStates.Num());

    // �ּ� ���� �ο� Ȯ��
    if (CurrentPlayerCount < MinPlayersToStart)
    {
        UE_LOG(LogTemp, Log, TEXT("LobbyGM: Not enough players to start (%d / %d)."), CurrentPlayerCount, MinPlayersToStart);
        return;
    }

    bool bAllReady = true;
    for (APlayerState* Player : LGS->PlayerArray)
    {
        if (!Player)
        {
            UE_LOG(LogTemp, Warning, TEXT("LobbyGM: Null PlayerState found in GameState PlayerArray."));
            bAllReady = false;
            break;
            // ����ó�� Null PlayerState�� �غ� �ȵ� ������ �����ؾ� �ǳ�? ����� �ϰ� ���� ����.
        }

        // �ʿ� �ش� �÷��̾ ��ϵǾ� �ִ���, ���°� true���� Ȯ��
        const bool* PlayerStatusInMap = PlayerReadyStates.Find(Player);
        if (PlayerStatusInMap == nullptr)
        {
            // GameState���� ������ �츮 �ʿ� ���� ��� (PostLogin���� ���� �߸��Ǿ��� �� ����) �� �ٽ� �ľ� �ϳ�?
            UE_LOG(LogTemp, Error, TEXT("LobbyGM: Player %s from GameState NOT FOUND in local PlayerReadyStates map. Assuming not ready."), *Player->GetPlayerName());
            bAllReady = false;
            break;
        }

        if (*PlayerStatusInMap == false)
        {
            bAllReady = false;
            UE_LOG(LogTemp, Log, TEXT("LobbyGM: Some Player 'Not Ready'."));
            break;
        }
    }

    if (bAllReady && CurrentPlayerCount > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("LobbyGM: All %d players are ready! Traveling to %s..."), CurrentPlayerCount, *NextLevelName.ToString());
        UWorld* World = GetWorld();
        if (World)
        {
            bUseSeamlessTravel = true;
            World->ServerTravel(NextLevelName.ToString() + "?listen", true);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("LobbyGM: World is null, cannot ServerTravel."));
        }
    }
    else
    {
        if (CurrentPlayerCount > 0) // �÷��̾ ���� ���� "���� �غ� �ȵ�" �α� ���
        {
            UE_LOG(LogTemp, Log, TEXT("LobbyGM: Not all players are ready yet or no players connected."));
        }
    }
}
