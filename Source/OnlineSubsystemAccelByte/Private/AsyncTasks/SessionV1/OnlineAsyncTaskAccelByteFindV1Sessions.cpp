// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteFindV1Sessions.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "Misc/DefaultValueHelper.h"
#include "OnlineSubsystemAccelByteDefines.h"

FOnlineAsyncTaskAccelByteFindV1Sessions::FOnlineAsyncTaskAccelByteFindV1Sessions(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InSearchingPlayerId, const TSharedRef<FOnlineSessionSearch>& InSearchSettings)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, SearchSettings(InSearchSettings)
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InSearchingPlayerId.AsShared());
}

void FOnlineAsyncTaskAccelByteFindV1Sessions::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	if (SearchSettings->SearchState != EOnlineAsyncTaskState::NotStarted)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Error, TEXT("Search is in a state other than 'NotStarted', cannot continue with finding sessions! Current search state: %s"), EOnlineAsyncTaskState::ToString(SearchSettings->SearchState));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	SearchSettings->SearchState = EOnlineAsyncTaskState::InProgress;
	SearchSettings->SearchResults.Empty();

	// We need a way to signal to the SessionBrowser APIs whether we want to search for P2P relay sessions, or for dedicated matches
	// Only way we can do this is by 
	FString SearchType;
	SearchSettings->QuerySettings.Get(SETTING_SEARCH_TYPE, SearchType);

	// Default to searching for dedicated sessions, P2P session search will be opt-in
	if (SearchType.IsEmpty())
	{
		SearchType = SETTING_SEARCH_TYPE_DEDICATED;
	}

	THandler<FAccelByteModelsSessionBrowserGetResult> OnSessionBrowserFindSuccessDelegate = THandler<FAccelByteModelsSessionBrowserGetResult>::CreateRaw(this, &FOnlineAsyncTaskAccelByteFindV1Sessions::OnSessionBrowserFindSuccess);
	FErrorHandler OnSessionBrowserFindErrorDelegate = FErrorHandler::CreateLambda([this](int32 ErrorCode, const FString& ErrorMessage) {
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		UE_LOG_AB(Error, TEXT("Failed to find sessions! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	});

	FString RegionName = TEXT("");
	SearchSettings->QuerySettings.Get(SETTING_REGION, RegionName);
	ApiClient->SessionBrowser.GetGameSessions(SearchType, FString(""), OnSessionBrowserFindSuccessDelegate, OnSessionBrowserFindErrorDelegate, 0, SearchSettings->MaxSearchResults);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent off task to find %s game sessions!"), *SearchType);
}

void FOnlineAsyncTaskAccelByteFindV1Sessions::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		SearchSettings->SearchState = EOnlineAsyncTaskState::Done;
		SearchSettings->SearchResults = SearchResults;
	}
	else
	{
		SearchSettings->SearchState = EOnlineAsyncTaskState::Failed;
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindV1Sessions::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	check(Subsystem != nullptr);
	const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	SessionInterface->TriggerOnFindSessionsCompleteDelegates(bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindV1Sessions::OnSessionBrowserFindSuccess(const FAccelByteModelsSessionBrowserGetResult& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Session Count: %d"), Result.Sessions.Num());

	if (SearchSettings->SearchState != EOnlineAsyncTaskState::InProgress)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Error, TEXT("Cannot set up session find results if the search is not InProgress! Current state: %s"), *EOnlineAsyncTaskState::ToString(SearchSettings->SearchState));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	// Update the timeout just in case processing takes a bit of time
	SetLastUpdateTimeToCurrentTime();

	for (const FAccelByteModelsSessionBrowserData& FoundSession : Result.Sessions)
	{
		FOnlineSessionSearchResult SearchResult;
		SearchResult.PingInMs = -1;
		
		FOnlineSession Session;

		// This block contains settings for features that we do not currently support, such as join via presence, invites, etc
		Session.NumOpenPrivateConnections = FoundSession.Game_session_setting.Current_internal_player;
		Session.SessionSettings.NumPrivateConnections = FoundSession.Game_session_setting.Max_internal_player;
		Session.SessionSettings.bAllowJoinViaPresence = false;
		Session.SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
		Session.SessionSettings.bAllowInvites = false;
		Session.SessionSettings.bUsesPresence = true;
		Session.SessionSettings.bAntiCheatProtected = false;
		// End unsupported feature block

		// # AB (Apin) splitgate do differently about player count calculation
		Session.NumOpenPublicConnections = FoundSession.Game_session_setting.Max_player - FoundSession.Game_session_setting.Current_player;
		FDefaultValueHelper::ParseInt(FoundSession.Game_version, Session.SessionSettings.BuildUniqueId);
		Session.SessionSettings.NumPublicConnections = FoundSession.Game_session_setting.Max_player;
		Session.SessionSettings.bAllowJoinInProgress = FoundSession.Game_session_setting.Allow_join_in_progress;
		// Differentiating between dedicated and p2p sessions through the session browser is done through a string that will either be
		// 'p2p' or 'dedicated'. With that in mind, just check in a case insensitive manner if the session is marked as 'dedicated'.
		Session.SessionSettings.bIsDedicated = (FoundSession.Session_type.Compare(TEXT("dedicated"), ESearchCase::IgnoreCase) == 0);
		Session.SessionSettings.bIsLANMatch = false;
		Session.SessionSettings.bShouldAdvertise = true;

		auto Setting = FoundSession.Game_session_setting.Settings;
		if(Setting.JsonObject.IsValid())
		{
			for(const auto &JValue : FoundSession.Game_session_setting.Settings.JsonObject->Values)
			{
				switch (JValue.Value->Type)
				{
				case EJson::String:						
					Session.SessionSettings.Set(FName(JValue.Key), JValue.Value->AsString());
					break;
				case EJson::Boolean:						
					Session.SessionSettings.Set(FName(JValue.Key), JValue.Value->AsBool());
					break;
				case EJson::Number:						
					Session.SessionSettings.Set(FName(JValue.Key), JValue.Value->AsNumber());
					break;
				default:
					break;
				}
			}
		}		

		TSharedPtr<FOnlineSessionInfoAccelByteV1> SessionInfo = MakeShared<FOnlineSessionInfoAccelByteV1>();
		SessionInfo->SetSessionId(FoundSession.Session_id);

		if(!FoundSession.Server.Ip.IsEmpty())
		{
			// HostAddr should always be instantiated to a proper default FInternetAddr instance, but just in case we will check if this is valid
			bool bIsIpValid = false;
			if (SessionInfo->GetHostAddr() != nullptr)
			{
				SessionInfo->GetHostAddr()->SetIp(*FoundSession.Server.Ip, bIsIpValid);
				SessionInfo->GetHostAddr()->SetPort(FoundSession.Server.Port);
			}
		}

		// For sessions that are P2P, there will be a user ID associated with the session for the user who owns this session
		// If this is set, then we should set the owner ID and username to that contained in the result, and also set the
		// remote ID for the session info to the user ID.
		//
		// @sessions There may be a case where dedicated servers could have "owning" users, but for now that isn't supported
		if (!FoundSession.User_id.IsEmpty())
		{
			// Create composite ID for the owning user
			FAccelByteUniqueIdComposite CompositeId;
			CompositeId.Id = FoundSession.User_id;

			// Create a shared ref for the user composite ID and set the session info
			TSharedRef<const FUniqueNetIdAccelByteUser> OwnerId = FUniqueNetIdAccelByteUser::Create(CompositeId).ToSharedRef();
			Session.OwningUserId = OwnerId;
			
			Session.OwningUserName = FoundSession.Username;
			if(FoundSession.Session_type == SETTING_SEARCH_TYPE_PEER_TO_PEER_RELAY)
			{
				SessionInfo->SetRemoteId(FoundSession.User_id);
			}			
		}

		Session.SessionInfo = SessionInfo;

		SearchResult.Session = Session;
		SearchResults.Add(SearchResult);
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
}
