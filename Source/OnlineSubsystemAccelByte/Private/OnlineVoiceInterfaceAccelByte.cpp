// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineVoiceInterfaceAccelByte.h"

#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "OnlineSubsystemUtils.h"
#include "VoiceEngineImpl.h"

#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"

FOnlineVoiceAccelByte::FOnlineVoiceAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
	: AccelByteSubsystem(InSubsystem)
{
}

bool FOnlineVoiceAccelByte::Init()
{
	GConfig->GetBool(TEXT("OnlineSubsystem"), TEXT("bHasVoiceEnabled"), bIsEnabled, GEngineIni);
	return FOnlineVoiceImpl::Init();
}

void FOnlineVoiceAccelByte::RegisterTalkers(const TArray<TSharedRef<const FUniqueNetId>>& Players)
{
	if (!bIsEnabled)
	{
		return;
	}
	for (const auto& Player : Players)
	{
		RegisterTalker(Player);
	}
}

void FOnlineVoiceAccelByte::RegisterTalker(const FUniqueNetIdRef Player)
{
	if (!bIsEnabled)
	{
		return;
	}

	if (AccelByteSubsystem->IsLocalPlayer(Player.Get()))
	{
		RegisterLocalTalker(0);
	}
	else
	{
		RegisterRemoteTalker(Player.Get());
	}
}

void FOnlineVoiceAccelByte::RegisterTalker(const FUniqueNetIdRef Player, const FNamedOnlineSession& SessionName)
{
	if (!bIsEnabled)
	{
		return;
	}

	RegisterTalker(Player);

#if AB_USE_V2_SESSIONS
	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(SessionName.SessionInfo);
	if (!SessionInfo.IsValid())
	{
		return;
	}

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		return;
	}

	TArray<FAccelByteModelsV2GameSessionTeam> Teams = SessionInfo->GetTeamAssignments();
	auto LocalPlayerId = IdentityInterface->GetUniquePlayerId(0);
	if (!LocalPlayerId.IsValid())
	{
		return;
	}

	int32 LocalPlayerTeamIndex = GetPlayerTeamIndex(Teams, LocalPlayerId.ToSharedRef());
	int32 NewPlayerTeamIndex = GetPlayerTeamIndex(Teams, Player);

	if (LocalPlayerTeamIndex != NewPlayerTeamIndex)
	{
		MuteRemoteTalker(0, Player.Get(), false);
	}
#endif
}

void FOnlineVoiceAccelByte::RemoveAllTalkers()
{
	if (!bIsEnabled)
	{
		return;
	}

	UnregisterLocalTalkers();
	RemoveAllRemoteTalkers();
}

bool FOnlineVoiceAccelByte::IsVoiceEnabled()
{
	return bIsEnabled;
}

int32 FOnlineVoiceAccelByte::GetPlayerTeamIndex(TArray<FAccelByteModelsV2GameSessionTeam> Teams, const FUniqueNetIdRef UserId)
{
#if AB_USE_V2_SESSIONS
	FUniqueNetIdAccelByteUserRef Id = FUniqueNetIdAccelByteUser::CastChecked(UserId);
	if (Id->IsValid())
	{
		return Teams.IndexOfByPredicate(
			[Id](const FAccelByteModelsV2GameSessionTeam& Team)
			{
				return Team.UserIDs.Contains(Id->GetAccelByteId());
			});
	}
#endif
	return INDEX_NONE;
}
