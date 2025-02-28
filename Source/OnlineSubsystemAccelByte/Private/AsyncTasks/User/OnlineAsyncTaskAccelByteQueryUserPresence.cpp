// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryUserPresence.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlinePresenceInterfaceAccelByte.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteLobbyApi.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteQueryUserPresence::FOnlineAsyncTaskAccelByteQueryUserPresence(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InTargetUserId, const IOnlinePresence::FOnPresenceTaskCompleteDelegate& InDelegate, int32 InLocalUserNum)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, TargetUserId(FUniqueNetIdAccelByteUser::CastChecked(InTargetUserId))
	, PresenceResult(MakeShared<FOnlineUserPresenceAccelByte>())
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
	THandler<FAccelByteModelsBulkUserStatusNotif> OnQueryUserPresenceSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsBulkUserStatusNotif>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryUserPresence::OnQueryUserPresenceSuccess);
	FErrorHandler OnQueryUserPresenceErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryUserPresence::OnQueryUserPresenceError);
	API_FULL_CHECK_GUARD(Lobby);
	Lobby->BulkGetUserPresenceV2(UsersToQuery, OnQueryUserPresenceSuccessDelegate, OnQueryUserPresenceErrorDelegate, false);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserPresence::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if(bWasSuccessful)
	{
		const FOnlinePresenceAccelBytePtr PresenceInterface = StaticCastSharedPtr<FOnlinePresenceAccelByte>(SubsystemPin->GetPresenceInterface());
		PresenceInterface->UpdatePresenceCache(TargetUserId->GetAccelByteId(), PresenceResult);
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserPresence::TriggerDelegates() 
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const IOnlinePresencePtr PresenceInterface = SubsystemPin->GetPresenceInterface();
	if (PresenceInterface.IsValid()) 
	{
		PresenceInterface->TriggerOnPresenceReceivedDelegates(TargetUserId.Get(), PresenceResult);
		Delegate.ExecuteIfBound(TargetUserId.Get(), bWasSuccessful);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserPresence::OnQueryUserPresenceError(int32 ErrorCode, const FString& ErrorMessage) 
{
	UE_LOG_AB(Warning, TEXT("Failed to query presence for user! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteQueryUserPresence::OnQueryUserPresenceSuccess(const FAccelByteModelsBulkUserStatusNotif& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Query User Presence succeeded"));

	FOnlineUserPresenceStatusAccelByte PresenceStatus;
	
	// We can only set status if we have data from the backend, if we don't have this, then the user is offline
	if (Result.Data.IsValidIndex(0))
	{
		PresenceStatus.StatusStr = Result.Data[0].Activity;
		PresenceStatus.SetPresenceStatus(Result.Data[0].Availability);
		if(!FDateTime::ParseIso8601(*Result.Data[0].LastSeenAt, PresenceResult->LastOnline))
		{
			UE_LOG_AB(Warning, TEXT("Unable to parse presence LastSeenAt from %s to FDateTime"), *Result.Data[0].LastSeenAt);
		}
	}

	PresenceResult->Status = PresenceStatus;

	if (Result.Data.Num() <= 0)
	{
		PresenceResult->bIsOnline = Result.Offline;
		PresenceResult->bIsPlayingThisGame = Result.Offline;
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Query User Presence succeeded, however no user presence data was obtained from user: %s. This user's presence may have not been queried recently. Marking user as offline."), *TargetUserId->ToDebugString());
	}
	else
	{
		PresenceResult->bIsOnline = Result.Online;
		PresenceResult->bIsPlayingThisGame = Result.Online;
		PresenceResult->Status.Properties.Add(DefaultPlatformKey, Result.Data[0].Platform);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
}