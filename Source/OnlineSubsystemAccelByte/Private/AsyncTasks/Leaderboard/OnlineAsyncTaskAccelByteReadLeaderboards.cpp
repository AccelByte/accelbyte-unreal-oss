// Copyright (c) 2023 - 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteReadLeaderboards.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineLeaderboardSystemAccelByte"

FOnlineAsyncTaskAccelByteReadLeaderboards::FOnlineAsyncTaskAccelByteReadLeaderboards(FOnlineSubsystemAccelByte* const InABInterface,
	int32 InLocalUserNum,
	const TArray<FUniqueNetIdRef>& InUsers, 
	FOnlineLeaderboardReadRef& InReadObject)
	: FOnlineAsyncTaskAccelByte(InABInterface, InLocalUserNum, true)
	, AccelByteUsers(InUsers)
	, LeaderboardObject(InReadObject)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Construct FOnlineAsyncTaskAccelByteReadLeaderboards"));

	LeaderboardObject->ReadState = EOnlineAsyncTaskState::InProgress;

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

FOnlineAsyncTaskAccelByteReadLeaderboards::FOnlineAsyncTaskAccelByteReadLeaderboards(FOnlineSubsystemAccelByte* const InABInterface, 
	int32 InLocalUserNum, 
	const TArray<FUniqueNetIdRef>& InUsers, 
	FOnlineLeaderboardReadRef& InReadObject, 
	bool bIsCycle,
	const FString& CycleId)
	: FOnlineAsyncTaskAccelByte(InABInterface, InLocalUserNum, true)
	, bUseCycle(bIsCycle)
	, CycleIdValue(CycleId)
	, AccelByteUsers(InUsers)
	, LeaderboardObject(InReadObject)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Construct FOnlineAsyncTaskAccelByteReadLeaderboards"));

	LeaderboardObject->ReadState = EOnlineAsyncTaskState::InProgress;

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReadLeaderboards::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialized"));

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		ErrorMessage = TEXT("request-failed-read-leaderboards-error-identity-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to read leaderboards, identity interface is invalid!"));
		return;
	}

	const FOnlineUserAccelBytePtr UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(SubsystemPin->GetUserInterface());
	if (!UserInterface.IsValid())
	{
		ErrorMessage = TEXT("request-failed-read-leaderboards-error-user-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to save leaderboard, User Interface is invalid!"));
		return;
	}

	const auto UserCacheInterface = SubsystemPin->GetUserCache();
	if (!UserCacheInterface.IsValid())
	{
		ErrorMessage = TEXT("request-failed-read-leaderboards-error-usercacheinterface-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to save leaderboard, UserCache Interface is invalid!"));
		return;
	}

	ELoginStatus::Type LoginStatus;
	LoginStatus = IdentityInterface->GetLoginStatus(LocalUserNum);

	if (LoginStatus != ELoginStatus::LoggedIn)
	{
		ErrorMessage = TEXT("request-failed-read-leaderboards-error-user-not-logged-in");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to read leaderboards, user not logged in!"));
		return;
	}
	
	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);
	if (IsDS.IsSet() && IsDS.GetValue()) // Is DedicatedServer
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to read leaderboards as this endpoint is not yet supported for dedicated server!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}
	
	// Get user info first, we need the display name
	TArray<FString> MissingUserAccounts;
	for(const FUniqueNetIdRef& AbId : AccelByteUsers)
	{
		TSharedPtr<FUserOnlineAccount> UserAccount = IdentityInterface->GetUserAccount(AbId);

		if(!UserAccount.IsValid())
		{
			TSharedRef<const FUniqueNetIdAccelByteUser> AccelByteUserRef = FUniqueNetIdAccelByteUser::CastChecked(AbId);
			MissingUserAccounts.Add(AccelByteUserRef->GetAccelByteId());
		}
	}

	if(MissingUserAccounts.Num() > 0)
	{
		bQueryUserAccountMissingRequired = true;

		// Query the user profile first
		auto QueryMissingUsersCompleted = TDelegateUtils<FOnQueryUsersComplete>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadLeaderboards::OnQueryMissingUsersCompleted);

		UserCacheInterface->QueryUsersByAccelByteIds(LocalUserNum, MissingUserAccounts, QueryMissingUsersCompleted, false);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
		return;
	}

	QueryLeaderboard();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReadLeaderboards::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Finalize"));

	LeaderboardObject->ReadState = bWasSuccessful ? EOnlineAsyncTaskState::Done : EOnlineAsyncTaskState::Failed;

	if (bWasSuccessful)
	{
		const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
		if (PredefinedEventInterface.IsValid())
		{
			FAccelByteModelsLeaderboardGetUsersRankingsPayload LeaderboardGetUsersRankingsPayload{};

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
			// LeaderboardName is an FString type in 5.5 and above
			LeaderboardGetUsersRankingsPayload.LeaderboardCode = LeaderboardObject->LeaderboardName;
#else
			LeaderboardGetUsersRankingsPayload.LeaderboardCode = LeaderboardObject->LeaderboardName.ToString();
#endif // ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5

			TArray<FString> FriendsUserIds{};
			for (int i = 0; i < AccelByteUsers.Num(); i++)
			{
				TSharedRef<const FUniqueNetIdAccelByteUser> ABUser = FUniqueNetIdAccelByteUser::CastChecked(AccelByteUsers[i]);
				FriendsUserIds.Add(ABUser->GetAccelByteId());
			}

			LeaderboardGetUsersRankingsPayload.UserId = UserId->GetAccelByteId();
			LeaderboardGetUsersRankingsPayload.TargetUserIds = FriendsUserIds;

			PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsLeaderboardGetUsersRankingsPayload>(LeaderboardGetUsersRankingsPayload));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReadLeaderboards::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineLeaderboardAccelBytePtr LeaderboardsInterface = StaticCastSharedPtr<FOnlineLeaderboardAccelByte>(SubsystemPin->GetLeaderboardsInterface());
	if (!ensure(LeaderboardsInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates to read leaderboards as our leaderboards interface is invalid!"));
		return;
	}

	LeaderboardsInterface->TriggerOnLeaderboardReadCompleteDelegates(bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReadLeaderboards::Tick()
{
	Super::Tick();

	// AsyncTask should be continue if query is required AND it's not complete yet
	if (bQueryUserAccountMissingRequired && !bQueryUserAccountMissingCompleted)
	{
		return;
	}

	if (CountRequests == 0)
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
}

void FOnlineAsyncTaskAccelByteReadLeaderboards::QueryLeaderboard()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("QueryLeaderboard"));
	
	auto CopyOfUserList = AccelByteUsers;

	// Iterate Every 20 Friends and Each Iteration AccelByteUsers Decrease
	while (CopyOfUserList.Num() > 0)
	{
		TArray<FString> QueryUserIds;
		int32 IDsToProcess = FMath::Min(LeaderboardUserIdsLimit, CopyOfUserList.Num());

		OnReadLeaderboardsSuccessHandler = TDelegateUtils<THandler<FAccelByteModelsBulkUserRankingDataV3>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadLeaderboards::OnReadLeaderboardsSuccess);
		OnReadLeaderboardsFailedHandler = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadLeaderboards::OnReadLeaderboardsFailed);

		// Take the user ids, with the maximum number of 20
		for (int32 i = 0; i < IDsToProcess; i++)
		{
			TSharedRef<const FUniqueNetIdAccelByteUser> ABUser = FUniqueNetIdAccelByteUser::CastChecked(CopyOfUserList[i]);
			QueryUserIds.Add(ABUser->GetAccelByteId());

			// for filter result later
			CurrentProcessedUsers.Add(ABUser->GetAccelByteId(), ABUser);
		}

		TRY_PIN_SUBSYSTEM();

		// Call the endpoint using the stored user ids.
		TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);
		if (IsDS.IsSet() && !IsDS.GetValue()) //Is GameClient
		{
			API_FULL_CHECK_GUARD(Leaderboard, ErrorMessage);
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
			// LeaderboardName is an FString type in 5.5 and above
			Leaderboard->GetBulkUserRankingV3(QueryUserIds, LeaderboardObject->LeaderboardName, OnReadLeaderboardsSuccessHandler, OnReadLeaderboardsFailedHandler);
#else
			Leaderboard->GetBulkUserRankingV3(QueryUserIds, LeaderboardObject->LeaderboardName.ToString(), OnReadLeaderboardsSuccessHandler, OnReadLeaderboardsFailedHandler);
#endif // ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
			CountRequests++;
		}
		else
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to read leaderboards as this endpoint is not yet supported for dedicated server!"));
			CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		}

		CopyOfUserList.RemoveAt(0, IDsToProcess);
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("QueryLeaderboard"));
}

void FOnlineAsyncTaskAccelByteReadLeaderboards::OnReadLeaderboardsSuccess(FAccelByteModelsBulkUserRankingDataV3 const& Result)
{
	FScopeLock ScopeLock(&OnReadLeaderboardSuccessMtx);

	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Read Leaderboards Success"));

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to save leaderboard data as our Identity Interface is invalid!"));
		return;
	}

	const FOnlineUserCacheAccelBytePtr UserCacheInterface = StaticCastSharedPtr<FOnlineUserCacheAccelByte>(SubsystemPin->GetUserCache());
	if (!UserCacheInterface.IsValid())
	{
		ErrorMessage = TEXT("request-failed-read-leaderboards-error-usercache-interface-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to read leaderboard, UserCache Interface is invalid!"));
		return;
	}

	const FOnlineLeaderboardAccelBytePtr LeaderboardsInterface = StaticCastSharedPtr<FOnlineLeaderboardAccelByte>(SubsystemPin->GetLeaderboardsInterface());
	if (!LeaderboardsInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to save leaderboard data as our Leaderboards Interface is invalid!"));
		return;
	}

	FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!LocalUserId.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to save leaderboard data as Local User is invalid!"));
		return;
	}

	for (const auto& BulkLeaderboardResult : Result.Data)
	{
		FUniqueNetIdPtr CurrentUserId;

		// Check if the returned user ids match with the request since we support partial request
		if (CurrentProcessedUsers.Contains(BulkLeaderboardResult.UserId))
		{
			CurrentUserId = *CurrentProcessedUsers.Find(BulkLeaderboardResult.UserId);
		}
		else
		{
			continue;
		}

		// Get the user info
		auto UserInfo = UserCacheInterface->GetUser(*CurrentUserId.Get());
		FString EntryDisplayName{};
		if (UserInfo.IsValid())
		{
			EntryDisplayName = UserInfo->DisplayName;
		}

		FOnlineStatsRow* LeaderboardRow = LeaderboardObject.Get().FindPlayerRecord(*CurrentUserId.Get());
		if (LeaderboardRow == NULL)
		{
			LeaderboardRow = &LeaderboardObject->Rows.Emplace_GetRef(FOnlineStatsRow(
				EntryDisplayName,
				FUniqueNetIdAccelByteUser::CastChecked(CurrentUserId.ToSharedRef())));
		}

		if (bUseCycle)
		{
			int32 CycleIndex = FindCycle(BulkLeaderboardResult.Cycles, CycleIdValue);

			if (CycleIndex != INDEX_NONE)
			{
				// Leaderboard cycle
				LeaderboardRow->Rank = BulkLeaderboardResult.Cycles[CycleIndex].Rank;

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
				// Column key uses FString in 5.5 and above
				LeaderboardRow->Columns.Add(TEXT("Cycle_Point"), BulkLeaderboardResult.Cycles[CycleIndex].Point);
#else
				LeaderboardRow->Columns.Add(FName(TEXT("Cycle_Point")), BulkLeaderboardResult.Cycles[CycleIndex].Point);
#endif // ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
			}
			else
			{
				AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to read leaderboards, Cycle Id is invalid! Leaderboard will return empty"));
			}
		}
		else
		{
			// Leaderboard all time
			LeaderboardRow->Rank = BulkLeaderboardResult.AllTime.Rank;

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
			// Column key uses FString in 5.5 and above
			LeaderboardRow->Columns.Add(TEXT("AllTime_Point"), BulkLeaderboardResult.AllTime.Point);
#else
			LeaderboardRow->Columns.Add(FName(TEXT("AllTime_Point")), BulkLeaderboardResult.AllTime.Point);
#endif // ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
		}

		// Put the leaderboard value into read leaderboard object reference (column meta data)
		for (const auto& ColumnMeta : LeaderboardObject->ColumnMetadata)
		{
			FVariantData* LastColumn = NULL;
			switch (ColumnMeta.DataType)
			{
				case EOnlineKeyValuePairDataType::Float:
				{
					float Value = BulkLeaderboardResult.AllTime.Point;
					LastColumn = &(LeaderboardRow->Columns.Add(ColumnMeta.ColumnName, FVariantData(Value)));
					bWasSuccessful = true;
					break;
				}

				default:
				{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
					// ColumnName is an FString type in 5.5 and above
					UE_LOG_ONLINE(Warning, TEXT("Unsupported key value pair during data retrieval %s"), *ColumnMeta.ColumnName);
#else
					UE_LOG_ONLINE(Warning, TEXT("Unsupported key value pair during data retrieval %s"), *ColumnMeta.ColumnName.ToString());
#endif
					break;
				}
			}
		}
	}

	if (CountRequests > 0)
	{
		CountRequests--;
	}
	else
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReadLeaderboards::OnQueryMissingUsersCompleted(bool bWasQuerySuccessful, TArray<FAccelByteUserInfoRef> Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Query Missing Users Completed"));
	bQueryUserAccountMissingCompleted = true;

	if (!bWasQuerySuccessful)
	{
		TaskOnlineError = EOnlineErrorResult::RequestFailure;
		TaskErrorStr = TEXT("failed-to-query-user-profile-before-query-leaderboard");
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("%s"), *TaskErrorStr);
		CompleteTask(EAccelByteAsyncTaskCompleteState::Incomplete);
		return;
	}

	QueryLeaderboard();
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReadLeaderboards::OnReadLeaderboardsFailed(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Warning, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);

	ErrorCode = FString::Printf(TEXT("%d"), Code);
	ErrorMessage = ErrMsg;

	CountRequests--;

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

int32 FOnlineAsyncTaskAccelByteReadLeaderboards::FindCycle(
	TArray<FAccelByteModelsCycleRank> const& Cycles, 
	FString const& CycleId)
{
	int32 CycleIndexResult = -1;

	for (int32 i = 0; i < Cycles.Num(); i++)
	{
		if (Cycles[i].CycleId == CycleId)
		{
			CycleIndexResult = i;
		}
	}

	return CycleIndexResult;
}

#undef ONLINE_ERROR_NAMESPACE