// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteCreateV2Party.h"
#include "OnlineSubsystemAccelByte.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Core/AccelByteRegistry.h"
#include "OnlineSubsystemSessionSettings.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"

FOnlineAsyncTaskAccelByteCreateV2Party::FOnlineAsyncTaskAccelByteCreateV2Party(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, const FOnlineSessionSettings& InNewSessionSettings)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, NewSessionSettings(InNewSessionSettings)
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InLocalUserId.AsShared());
}

void FOnlineAsyncTaskAccelByteCreateV2Party::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	// First, check if we are in a party locally, if so then we should fail and say to call leave party
	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	ensure(SessionInterface.IsValid());

	JoinType = SessionInterface->GetJoinabiltyFromSessionSettings(NewSessionSettings);
	if (JoinType != EAccelByteV2SessionJoinability::EMPTY && JoinType != EAccelByteV2SessionJoinability::INVITE_ONLY)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Attempted to override party joinability with a value other than INVITE_ONLY! Parties are only able to be invite only!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// Next, we want to send off a request to check on the backend if we are in a party. This way we can validate in case
	// we're in one, but we haven't restored our state. This will tell the developer to call RestoreParties to restore
	// that previous state and act accordingly.
	THandler<FAccelByteModelsV2PaginatedPartyQueryResult> OnGetMyPartiesSuccessDelegate = THandler<FAccelByteModelsV2PaginatedPartyQueryResult>::CreateRaw(this, &FOnlineAsyncTaskAccelByteCreateV2Party::OnGetMyPartiesSuccess);
	FErrorHandler OnGetMyPartiesErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteCreateV2Party::OnGetMyPartiesError);
	ApiClient->Session.GetMyParties(OnGetMyPartiesSuccessDelegate, OnGetMyPartiesErrorDelegate); // Querying for all connected statuses here, will filter in delegate handler if needed

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent request to get info about current party before creating party!"));
}

void FOnlineAsyncTaskAccelByteCreateV2Party::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	ensure(SessionInterface.IsValid());

	if (bWasSuccessful)
	{
		SessionInterface->FinalizeCreatePartySession(SessionName, PartyInfo);
	}
	else
	{
		SessionInterface->RemoveNamedSession(SessionName);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateV2Party::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	if (SessionInterface.IsValid())
	{
		SessionInterface->TriggerOnCreateSessionCompleteDelegates(SessionName, bWasSuccessful);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateV2Party::OnGetMyPartiesSuccess(const FAccelByteModelsV2PaginatedPartyQueryResult& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("PlayerInParty: %s"), LOG_BOOL_FORMAT(Result.Data.Num() > 0));

	// Filter the resulting array by making sure our status is joined or connected, otherwise we won't be able to create a
	// party due to having invites pending for other parties
	TArray<FAccelByteModelsV2PartySession> ActivePartySessions = Result.Data.FilterByPredicate([UserIdStr = UserId->GetAccelByteId()](const FAccelByteModelsV2PartySession& Session) {
		const FAccelByteModelsV2SessionUser* FoundMember = Session.Members.FindByPredicate([UserIdStr](const FAccelByteModelsV2SessionUser& Member) {
			return Member.ID == UserIdStr;
		});

		if (FoundMember != nullptr)
		{
			return FoundMember->Status == EAccelByteV2SessionMemberStatus::JOINED || FoundMember->Status == EAccelByteV2SessionMemberStatus::CONNECTED;
		}

		return false;
	});

	if (ActivePartySessions.Num() > 0)
	{
		// If we successfully get non-invited party info then we have to fail the task since we are already in a party
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create party session for user (%s) as they are already in a party on the backend! Call RestoreParties then call LeaveParty before trying to create a new one!"), *UserId->ToDebugString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// Since we are not in a party, we can send the request to create one
	FAccelByteModelsV2PartyCreateRequest CreatePartyRequest;

	if (JoinType != EAccelByteV2SessionJoinability::EMPTY)
	{
		CreatePartyRequest.Joinability = JoinType;
	}
		
	// Ensure that we have a session template set and associate it with the create party request
	if (!NewSessionSettings.Get(SETTING_SESSION_TEMPLATE_NAME, CreatePartyRequest.ConfigurationName))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create party session as a session template was not provided! A session setting must be present for SETTING_SESSION_TEMPLATE_NAME associated with a valid session template on the backend!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	ensure(SessionInterface.IsValid());

	CreatePartyRequest.Attributes.JsonObject = SessionInterface->ConvertSessionSettingsToJsonObject(NewSessionSettings);

	int32 MinimumPlayers = 0;
	if (NewSessionSettings.Get(SETTING_SESSION_MINIMUM_PLAYERS, MinimumPlayers) && MinimumPlayers > 0)
	{
		CreatePartyRequest.MinPlayers = MinimumPlayers;
	}

	int32 MaximumPlayers = SessionInterface->GetSessionMaxPlayerCount(SessionName);
	if (MinimumPlayers > 0)
	{
		CreatePartyRequest.MaxPlayers = MaximumPlayers;
	}

	int32 InactiveTimeout = 0;
	if (NewSessionSettings.Get(SETTING_SESSION_INACTIVE_TIMEOUT, InactiveTimeout) && InactiveTimeout > 0)
	{
		CreatePartyRequest.InactiveTimeout = InactiveTimeout;
	}

	int32 InviteTimeout = 0;
	if (NewSessionSettings.Get(SETTING_SESSION_INVITE_TIMEOUT, InviteTimeout) && InviteTimeout > 0)
	{
		CreatePartyRequest.InviteTimeout = InviteTimeout;
	}

	// Add self to members array
	FAccelByteModelsV2SessionUser SelfMember;
	SelfMember.ID = UserId->GetAccelByteId();
	SelfMember.PlatformID = Subsystem->GetSimplifiedNativePlatformName(UserId->GetPlatformType());
	SelfMember.PlatformUserID = UserId->GetPlatformId();
	CreatePartyRequest.Members.Emplace(SelfMember);

	THandler<FAccelByteModelsV2PartySession> OnCreatePartySuccessDelegate = THandler<FAccelByteModelsV2PartySession>::CreateRaw(this, &FOnlineAsyncTaskAccelByteCreateV2Party::OnCreatePartySuccess);
	FErrorHandler OnCreatePartyErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteCreateV2Party::OnCreatePartyError);
	ApiClient->Session.CreateParty(CreatePartyRequest, OnCreatePartySuccessDelegate, OnCreatePartyErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent off request to create party session for user '%s'!"), *UserId->ToDebugString());
}

void FOnlineAsyncTaskAccelByteCreateV2Party::OnGetMyPartiesError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create party session for user '%s' as the call to get information about the current user's party the backend failed! Error code: %d; Error message: %s"), *UserId->ToDebugString(), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteCreateV2Party::OnCreatePartySuccess(const FAccelByteModelsV2PartySession& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	// CreateParty response also has information about the party, so just copy the struct to the member so that we
	// can construct the party object in the Finalize method
	PartyInfo = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateV2Party::OnCreatePartyError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to create party session from backend! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}