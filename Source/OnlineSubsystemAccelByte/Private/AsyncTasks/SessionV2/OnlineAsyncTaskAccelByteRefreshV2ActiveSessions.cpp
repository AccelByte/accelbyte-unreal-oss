// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRefreshV2ActiveSessions.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteRefreshV2ActiveSessions::FOnlineAsyncTaskAccelByteRefreshV2ActiveSessions(FOnlineSubsystemAccelByte* const InABInterface,
	const int32 InLocalUserNum,
	const TArray<FName>& InSessionNames,
	bool bOnReconnectedRefreshSessionDelegates)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, LocalUserNum(InLocalUserNum)
	, SessionNames(InSessionNames)
	, bTriggerOnReconnectedRefreshSessionDelegates(bOnReconnectedRefreshSessionDelegates)
{
}

void FOnlineAsyncTaskAccelByteRefreshV2ActiveSessions::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Total Session Nums: %d"), SessionNames.Num());

	const TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	check(SessionInterface.IsValid());

	TotalSessionRefreshed.Reset();
	
	FOnRefreshSessionComplete OnRefreshSessionCompleteDelegate = TDelegateUtils<FOnRefreshSessionComplete>::CreateThreadSafeSelfPtr(this,
		&FOnlineAsyncTaskAccelByteRefreshV2ActiveSessions::HandleOnRefreshSessionComplete);

	TArray<FName> SessionsTest{};
	for (auto SessionName : SessionNames)
	{
		bool bIsRefreshSession{true};
		
		const EAccelByteV2SessionType SessionType = SessionInterface->GetSessionTypeFromSettings(SessionInterface->GetNamedSession(SessionName)->SessionSettings);
		if (SessionType == EAccelByteV2SessionType::Unknown)
		{
			UE_LOG_AB(Warning, TEXT("Could not update session as the session's type is neither Game nor Party!"))
			bIsRefreshSession = false;
		}
		else if (SessionType == EAccelByteV2SessionType::PartySession && IsRunningDedicatedServer())
		{
			UE_LOG_AB(Warning, TEXT("Game servers are not able to refresh party sessions!"))
			bIsRefreshSession = false;
		}

		//TODO make the current async task as EPIC TASK & this RefreshSession should become a child task
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

void FOnlineAsyncTaskAccelByteRefreshV2ActiveSessions::Finalize()
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

void FOnlineAsyncTaskAccelByteRefreshV2ActiveSessions::TriggerDelegates()
{
	if (!bTriggerOnReconnectedRefreshSessionDelegates)
	{
		return;
	}

	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s. Total RemovedSessionNames: %d"), LOG_BOOL_FORMAT(bWasSuccessful), RemovedSessionNames.Num());


	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid()))
	{
		FString ErrorString = TEXT("Failed to trigger delegates to refresh-active-session-after-reconnected as AccelByte session interface is invalid!");
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("%s"), *ErrorString);
		return;
	}

	SessionInterface->TriggerAccelByteOnReconnectedRefreshSessionDelegates(
		LocalUserNum,
		bWasSuccessful,
		RemovedSessionNames);

	if (!bWasSuccessful)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to refresh active session reconnected."));
		return;
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRefreshV2ActiveSessions::HandleOnRefreshSessionComplete(bool bResponseSuccess)
{
	TotalSessionRefreshed.Increment();
	if (SessionNames.Num() == TotalSessionRefreshed.GetValue())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
}