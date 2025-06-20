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
#include "OnlineSubsystemAccelByteConfig.h"

FOnlineVoiceAccelByte::FOnlineVoiceAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
#if ENGINE_MAJOR_VERSION >= 5
	: AccelByteSubsystem(InSubsystem->AsWeak())
#else
	: AccelByteSubsystem(InSubsystem->AsShared())
#endif
{}

bool FOnlineVoiceAccelByte::Init()
{
	// #NOTE Normally, we would want to check config directly to allow for on the fly changing of values. However, for
	// voice the system would need to be reinitialized. So for this case, just check during init.
	FOnlineSubsystemAccelBytePtr SubsystemPin = AccelByteSubsystem.Pin();
	if (SubsystemPin.IsValid())
	{
		FOnlineSubsystemAccelByteConfigPtr Config = SubsystemPin->GetConfig();
		if (Config.IsValid())
		{
			bIsEnabled = Config->GetVoiceEnabled().GetValue();
		}
	}

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

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}
	
	if (AccelByteSubsystemPtr->IsLocalPlayer(Player.Get()))
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

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return;
	}

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
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
