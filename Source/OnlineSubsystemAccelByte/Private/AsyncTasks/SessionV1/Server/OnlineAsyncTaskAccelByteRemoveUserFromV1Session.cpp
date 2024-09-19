// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRemoveUserFromV1Session.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "Core/AccelByteUtilities.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteRemoveUserFromV1Session::FOnlineAsyncTaskAccelByteRemoveUserFromV1Session(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InChannelName, const FString& InMatchId, const FOnRemoveUserFromSessionComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, ChannelName(InChannelName)
	, MatchId(InMatchId)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteRemoveUserFromV1Session::Initialize()
{
	TRY_PIN_SUBSYSTEM()

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("ChannelName: %s"), *ChannelName);

	if (!IsRunningDedicatedServer())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to remove user from session as the current game instance is not a dedicated server!"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to remove user from session as our identity interface was invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	OnRemoveUserFromSessionSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRemoveUserFromV1Session::OnRemoveUserFromSessionSuccess);
	OnRemoveUserFromSessionErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRemoveUserFromV1Session::OnRemoveUserFromSessionError);

	if (!IdentityInterface->IsServerAuthenticated())
	{
		const FOnAuthenticateServerComplete OnAuthenticateServerCompleteDelegate = TDelegateUtils<FOnAuthenticateServerComplete>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteRemoveUserFromV1Session::OnAuthenticateServerComplete);
		IdentityInterface->AuthenticateAccelByteServer(OnAuthenticateServerCompleteDelegate);

		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Authenticating server with client credentials to get session information"));
		return;
	}

	FServerApiClientPtr ServerApiClient = FMultiRegistry::GetServerApiClient();
	ServerApiClient->ServerMatchmaking.RemoveUserFromSession(ChannelName, MatchId, *UserId->GetAccelByteId(), OnRemoveUserFromSessionSuccessDelegate, OnRemoveUserFromSessionErrorDelegate);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRemoveUserFromV1Session::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRemoveUserFromV1Session::OnAuthenticateServerComplete(bool bAuthenticationSuccessful)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bAuthenticationSuccessful: %s"), LOG_BOOL_FORMAT(bAuthenticationSuccessful));

	if (!bAuthenticationSuccessful)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to remove user from session with channel name '%s' as we failed to authenticate our server!"), *ChannelName);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	FServerApiClientPtr ServerApiClient = FMultiRegistry::GetServerApiClient();
	ServerApiClient->ServerMatchmaking.RemoveUserFromSession(ChannelName, MatchId, *UserId->GetAccelByteId(), OnRemoveUserFromSessionSuccessDelegate, OnRemoveUserFromSessionErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRemoveUserFromV1Session::OnRemoveUserFromSessionSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("ChannelName: %s"), *ChannelName);

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent off async task to register created dedicated game session"));
}

void FOnlineAsyncTaskAccelByteRemoveUserFromV1Session::OnRemoveUserFromSessionError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("'%s' Error code: %d; Error message: %s"), *ChannelName, ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

