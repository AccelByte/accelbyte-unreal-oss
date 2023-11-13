﻿// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteReadLeaderboardAroundUser.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteReadLeaderboardAroundUser::FOnlineAsyncTaskAccelByteReadLeaderboardAroundUser(
	FOnlineSubsystemAccelByte* const InABInterface,
	int32 InLocalUserNum,
	const FUniqueNetIdRef& UserId,
	const FOnlineLeaderboardReadRef& InReadObject,
	const int32 InRange,
	bool InUseCycle,
	const FString& InCycleId)
	: FOnlineAsyncTaskAccelByte(InABInterface, InLocalUserNum, true)
	, Range(InRange)
	, LeaderboardObject(InReadObject)
	, User(UserId)
	, bUseCycle(InUseCycle)
	, CycleId(InCycleId)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Construct FOnlineAsyncTaskAccelByteReadLeaderboardAroundUser"));

	LeaderboardObject->ReadState = EOnlineAsyncTaskState::InProgress;

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReadLeaderboardAroundUser::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialized"));
	
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		ErrorMessage = TEXT("request-failed-read-leaderboards-error-identity-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to read leaderboards, identity interface is invalid!"));
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

	OnGetUserRankingSuccessHandler = TDelegateUtils<THandler<FAccelByteModelsUserRankingDataV3>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadLeaderboardAroundUser::OnGetUserRankingSuccess);
	OnRequestFailedHandler = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadLeaderboardAroundUser::OnRequestFailed);

	const TSharedRef<const FUniqueNetIdAccelByteUser> ABUser = FUniqueNetIdAccelByteUser::CastChecked(User);
	const FString LeaderboardCode = LeaderboardObject->LeaderboardName.ToString();
	
	ApiClient->Leaderboard.GetUserRankingV3(UserId->GetAccelByteId(), LeaderboardCode, OnGetUserRankingSuccessHandler, OnRequestFailedHandler);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReadLeaderboardAroundUser::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Finalize"));

	LeaderboardObject->ReadState = bWasSuccessful ? EOnlineAsyncTaskState::Done : EOnlineAsyncTaskState::Failed;

	if (bWasSuccessful)
	{
		const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = Subsystem->GetPredefinedEventInterface();
		if (PredefinedEventInterface.IsValid())
		{
			FAccelByteModelsLeaderboardGetUserRankingPayload LeaderboardGetUserRankingPayload{};
			LeaderboardGetUserRankingPayload.LeaderboardCode = LeaderboardObject->LeaderboardName.ToString();
			LeaderboardGetUserRankingPayload.UserId = UserId->GetAccelByteId();

			PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsLeaderboardGetUserRankingPayload>(LeaderboardGetUserRankingPayload));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReadLeaderboardAroundUser::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineLeaderboardAccelBytePtr LeaderboardsInterface = StaticCastSharedPtr<FOnlineLeaderboardAccelByte>(Subsystem->GetLeaderboardsInterface());
	if (!ensure(LeaderboardsInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates to read leaderboards as our leaderboards interface is invalid!"));
		return;
	}

	LeaderboardsInterface->TriggerOnLeaderboardReadCompleteDelegates(bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReadLeaderboardAroundUser::Tick()
{
	Super::Tick();
}

void FOnlineAsyncTaskAccelByteReadLeaderboardAroundUser::OnRequestFailed(int32 Code, FString const& Message)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Warning, TEXT("Code: %d; Message: %s"), Code, *Message);

	ErrorCode = FString::Printf(TEXT("%d"), Code);
	ErrorMessage = Message;
	bWasSuccessful = false;

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReadLeaderboardAroundUser::OnGetUserRankingSuccess(FAccelByteModelsUserRankingDataV3 const& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Get User Rank Success"));

	int32 UserRank = Result.AllTime.Rank;
	const bool bCycleExist = GetUserCycleRank(Result, UserRank);
	
	if(bUseCycle && !bCycleExist)
	{
		ErrorMessage = TEXT("User is not part of the cycle");
		bWasSuccessful = false;

		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));

		return;
	}
	
	const int32 Limit = Range * 2;
	const FString LeaderboardCode = LeaderboardObject->LeaderboardName.ToString();
	Offset = UserRank - Range;

	Offset = Offset < 0 ? 0 : Offset;

	OnGetRankingSuccessHandler = TDelegateUtils<THandler<FAccelByteModelsLeaderboardRankingResultV3>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadLeaderboardAroundUser::OnGetRankingSuccess);

	if(bUseCycle)
	{
		ApiClient->Leaderboard.GetRankingByCycle(
			LeaderboardCode,
			CycleId,
			Offset,
			Limit,
			OnGetRankingSuccessHandler,
			OnRequestFailedHandler);
	}else
	{
		ApiClient->Leaderboard.GetRankingsV3(
			LeaderboardCode, 
			Offset,
			Limit,
			OnGetRankingSuccessHandler,
			OnRequestFailedHandler);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}


void FOnlineAsyncTaskAccelByteReadLeaderboardAroundUser::OnGetRankingSuccess(FAccelByteModelsLeaderboardRankingResultV3 const& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Get Rank User"));

	int32 Count = 1;
	for(const FAccelByteModelsUserPoint& Data : Result.Data)
	{
		FUniqueNetIdAccelByteUserRef UniqueNetIdAccelByteUserRef = FUniqueNetIdAccelByteUser::Create(Data.userId);
		FUniqueNetIdRef UniqueNetIdRef = StaticCastSharedRef<const FUniqueNetId>(UniqueNetIdAccelByteUserRef);
		
		FOnlineStatsRow* LeaderboardRow = new (LeaderboardObject->Rows)
				FOnlineStatsRow(
					Data.userId,
					UniqueNetIdRef);

		LeaderboardRow->Rank = Offset + Count;
		LeaderboardRow->Columns.Add(FName("Point"), Data.point);

		Count++;
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	bWasSuccessful = true;

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

bool FOnlineAsyncTaskAccelByteReadLeaderboardAroundUser::GetUserCycleRank(FAccelByteModelsUserRankingDataV3 const& RanksData, int32& OutRank) const
{
	bool bCycleExist = false;
	
	if(bUseCycle)
	{
		for(FAccelByteModelsCycleRank const& Data : RanksData.Cycles)
		{
			if(Data.CycleId.Equals(CycleId))
			{
				bCycleExist = true;
				OutRank = Data.Rank;
			}
		}
	}else
	{
		OutRank = RanksData.AllTime.Rank;
		bCycleExist = true;
	}

	return bCycleExist;
}


