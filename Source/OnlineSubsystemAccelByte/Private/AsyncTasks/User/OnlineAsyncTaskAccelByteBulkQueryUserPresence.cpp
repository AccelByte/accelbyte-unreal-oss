// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteBulkQueryUserPresence.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlinePresenceInterfaceAccelByte.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteLobbyApi.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteBulkQueryUserPresence::FOnlineAsyncTaskAccelByteBulkQueryUserPresence(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InUserId, const TArray<FUniqueNetIdRef>& InTargetUserIds)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
	
	for(const auto& InTargetUserId : InTargetUserIds)
	{
		TargetUserIds.Emplace(FUniqueNetIdAccelByteUser::CastChecked(InTargetUserId));
	}
}

void FOnlineAsyncTaskAccelByteBulkQueryUserPresence::Initialize() 
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(" "));

	if(TargetUserIds.Num() < 1)
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to query user presence, UserIds are empty"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	// Construct an array to use for a bulk presence query, in this case this will just be the target user ID
	TArray<FString> UsersToQuery;
	for(const auto& TargetUserId : TargetUserIds)
	{
		UsersToQuery.Emplace(TargetUserId->GetAccelByteId());
	}

	// Send off the actual request to get user presence
	THandler<FAccelByteModelsBulkUserStatusNotif> OnQueryUserPresenceSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsBulkUserStatusNotif>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteBulkQueryUserPresence::OnQueryUserPresenceSuccess);
	FErrorHandler OnQueryUserPresenceErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteBulkQueryUserPresence::OnQueryUserPresenceError);
	API_CLIENT_CHECK_GUARD();
	ApiClient->Lobby.BulkGetUserPresenceV2(UsersToQuery, OnQueryUserPresenceSuccessDelegate, OnQueryUserPresenceErrorDelegate, false);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBulkQueryUserPresence::Finalize()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if(bWasSuccessful)
	{
		const FOnlinePresenceAccelBytePtr PresenceInterface = StaticCastSharedPtr<FOnlinePresenceAccelByte>(SubsystemPin->GetPresenceInterface());
		PresenceInterface->UpdatePresenceCache(PresenceResult);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBulkQueryUserPresence::TriggerDelegates() 
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlinePresenceAccelBytePtr PresenceInterface = StaticCastSharedPtr<FOnlinePresenceAccelByte>(SubsystemPin->GetPresenceInterface());
	if (PresenceInterface.IsValid()) 
	{
		PresenceInterface->TriggerOnBulkQueryPresenceCompleteDelegates(bWasSuccessful, PresenceResult);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBulkQueryUserPresence::OnQueryUserPresenceError(int32 ErrorCode, const FString& ErrorMessage) 
{
	UE_LOG_AB(Warning, TEXT("Failed to query presence for user! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteBulkQueryUserPresence::OnQueryUserPresenceSuccess(const FAccelByteModelsBulkUserStatusNotif& Result) {
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Query User Presence succeeded"));
	
	// We can only set status if we have data from the backend, if we don't have this, then the user is offline
	for(const auto& PresenceData : Result.Data)
	{
		FOnlineUserPresenceStatusAccelByte PresenceStatus;
		
		PresenceStatus.StatusStr = PresenceData.Activity;
		PresenceStatus.SetPresenceStatus(PresenceData.Availability);

		TSharedPtr<FOnlineUserPresenceAccelByte> Presence = MakeShared<FOnlineUserPresenceAccelByte>();
		
		Presence->Status = static_cast<FOnlineUserPresenceStatus>(PresenceStatus);
		Presence->bIsOnline = PresenceData.Availability == EAvailability::Online;
		Presence->bIsPlayingThisGame = PresenceData.Availability == EAvailability::Online;
		Presence->Status.Properties.Add(DefaultPlatformKey, PresenceData.Platform);
		if(!FDateTime::ParseIso8601(*PresenceData.LastSeenAt, Presence->LastOnline))
		{
			UE_LOG_AB(Warning, TEXT("Failed to parse LastOnline FDateTime from Presence LastSeenAt %s for user %s"), *PresenceData.LastSeenAt, *PresenceData.UserID);
		}

		PresenceResult.Emplace(PresenceData.UserID, Presence.ToSharedRef());
	}

	if (Result.NotProcessed.Num() > 0)
	{
		THandler<FAccelByteModelsBulkUserStatusNotif> OnQueryUserPresenceSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsBulkUserStatusNotif>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteBulkQueryUserPresence::OnQueryUserPresenceSuccess);
		FErrorHandler OnQueryUserPresenceErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteBulkQueryUserPresence::OnQueryUserPresenceError);
		API_CLIENT_CHECK_GUARD();
		ApiClient->Lobby.BulkGetUserPresenceV2(Result.NotProcessed, OnQueryUserPresenceSuccessDelegate, OnQueryUserPresenceErrorDelegate, false);
	}
	else
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}