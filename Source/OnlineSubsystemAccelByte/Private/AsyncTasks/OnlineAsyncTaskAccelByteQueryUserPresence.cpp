// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryUserPresence.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlinePresenceInterfaceAccelByte.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteLobbyApi.h"

FOnlineAsyncTaskAccelByteQueryUserPresence::FOnlineAsyncTaskAccelByteQueryUserPresence(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InTargetUserId, const IOnlinePresence::FOnPresenceTaskCompleteDelegate& InDelegate, int32 InLocalUserNum)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, TargetUserId(StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InTargetUserId.AsShared()))
	, LocalCachedPresence(MakeShared<FOnlineUserPresenceAccelByte>())
	, Delegate(InDelegate)
{
	LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteQueryUserPresence::Initialize() 
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(" "));

	// Construct an array to use for a bulk presence query, in this case this will just be the target user ID
	TArray<FString> UsersToQuery;
	UsersToQuery.Add(TargetUserId->GetAccelByteId());

	// Send off the actual request to get user presence
	THandler<FAccelByteModelsBulkUserStatusNotif> OnQueryUserPresenceSuccessDelegate = THandler<FAccelByteModelsBulkUserStatusNotif>::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryUserPresence::OnQueryUserPresenceSuccess);
	FErrorHandler OnQueryUserPresenceErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryUserPresence::OnQueryUserPresenceError);
	ApiClient->Lobby.BulkGetUserPresence(UsersToQuery, OnQueryUserPresenceSuccessDelegate, OnQueryUserPresenceErrorDelegate, false);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserPresence::Finalize() 
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful) 
	{
		// #AB #TODO (Voltaire) set cached presence in AccelBytePresenceInterface here?
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserPresence::TriggerDelegates() 
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const IOnlinePresencePtr PresenceInterface = Subsystem->GetPresenceInterface();
	if (PresenceInterface.IsValid()) 
	{
		PresenceInterface->TriggerOnPresenceReceivedDelegates(TargetUserId.Get(), LocalCachedPresence);
		Delegate.ExecuteIfBound(TargetUserId.Get(), bWasSuccessful);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserPresence::OnQueryUserPresenceError(int32 ErrorCode, const FString& ErrorMessage) 
{
	UE_LOG_AB(Warning, TEXT("Failed to query presence for user! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteQueryUserPresence::OnQueryUserPresenceSuccess(const FAccelByteModelsBulkUserStatusNotif& Result) {
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Query User Presence succeeded"));

	LocalCachedPresence = StaticCastSharedPtr<FOnlinePresenceAccelByte>(Subsystem->GetPresenceInterface())->FindOrCreatePresence(TargetUserId);
	FOnlineUserPresenceStatus PresenceStatus;
	
	// We can only set status if we have data from the backend, if we don't have this, then the user is offline
	if (Result.Data.IsValidIndex(0))
	{
		PresenceStatus.StatusStr = Result.Data[0].Activity;
	}

	LocalCachedPresence->Status = PresenceStatus;
	LocalCachedPresence->bIsOnline = Result.Online;
	LocalCachedPresence->bIsPlayingThisGame = Result.Online;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	if (Result.Data.Num() <= 0)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Query User Presence succeeded, however no user presence data was obtained from user: %s. This user's presence may have not been queried recently. Marking user as offline."), *TargetUserId->ToDebugString());
	}
	else
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
	}
}