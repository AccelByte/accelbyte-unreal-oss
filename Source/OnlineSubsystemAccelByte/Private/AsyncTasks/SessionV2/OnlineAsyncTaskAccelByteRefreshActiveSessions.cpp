// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRefreshActiveSessions.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteRefreshActiveSessions::FOnlineAsyncTaskAccelByteRefreshActiveSessions(FOnlineSubsystemAccelByte* const InABInterface
	, const TArray<FName>& InSessionNames
	, const FOnRefreshActiveSessionsComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface
		, INVALID_CONTROLLERID
		, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::UseTimeout) + ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
	, SessionNames(InSessionNames)
	, Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteRefreshActiveSessions::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Total Session Nums: %d"), SessionNames.Num());

	TRY_PIN_SUBSYSTEM();

	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);

	if (!IsDS.IsSet())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	check(SessionInterface.IsValid());

	TotalSessionRefreshed.Reset();
	
	FOnRefreshSessionComplete OnRefreshSessionCompleteDelegate = TDelegateUtils<FOnRefreshSessionComplete>::CreateThreadSafeSelfPtr(this
		, &FOnlineAsyncTaskAccelByteRefreshActiveSessions::HandleOnRefreshSessionComplete);

	TArray<FName> SessionsTest;
	for (auto SessionName : SessionNames)
	{
		bool bIsRefreshSession{true};
		
		const EAccelByteV2SessionType SessionType = SessionInterface->GetSessionTypeFromSettings(SessionInterface->GetNamedSession(SessionName)->SessionSettings);
		if (SessionType == EAccelByteV2SessionType::Unknown)
		{
			UE_LOG_AB(Warning, TEXT("Could not update session as the session's type is neither Game nor Party!"))
			bIsRefreshSession = false;
		}
		else if (SessionType == EAccelByteV2SessionType::PartySession && IsDS.GetValue())
		{
			UE_LOG_AB(Warning, TEXT("Game servers are not able to refresh party sessions!"))
			bIsRefreshSession = false;
		}

		if (bIsRefreshSession)
		{
			SessionInterface->RefreshSession(SessionName, OnRefreshSessionCompleteDelegate);
		}
		else
		{
			OnRefreshSessionCompleteDelegate.ExecuteIfBound(false);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRefreshActiveSessions::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();

	const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	check(SessionInterface.IsValid());

	for (auto SessionName : SessionNames)
	{
		FNamedOnlineSession* OnlineSession = SessionInterface->GetNamedSession(SessionName);
		if (OnlineSession == nullptr)
		{
			RemovedSessionNames.Emplace(SessionName);
		}
	}
}

void FOnlineAsyncTaskAccelByteRefreshActiveSessions::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s. Total RemovedSessionNames: %d"), LOG_BOOL_FORMAT(bWasSuccessful), RemovedSessionNames.Num());

	Delegate.ExecuteIfBound(bWasSuccessful, RemovedSessionNames);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRefreshActiveSessions::HandleOnRefreshSessionComplete(bool /*bWasSuccessful*/)
{
	TotalSessionRefreshed.Increment();
	if (SessionNames.Num() == TotalSessionRefreshed.GetValue())
	{
		bWasSuccessful = true;
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
}