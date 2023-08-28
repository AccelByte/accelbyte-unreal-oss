// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryUserAchievements.h"

#include "Interfaces/OnlineAchievementsInterface.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteQueryUserAchievements::FOnlineAsyncTaskAccelByteQueryUserAchievements(
	FOnlineSubsystemAccelByte* const InABSubsystem,
	FUniqueNetId const& InPlayerId,
	FOnQueryAchievementsCompleteDelegate const& InDelegate)
	:FOnlineAsyncTaskAccelByte(InABSubsystem),Delegate(InDelegate),bWasSuccessful(false)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InPlayerId);
}

void FOnlineAsyncTaskAccelByteQueryUserAchievements::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FOnlineAsyncTaskAccelByte::Initialize();
	QueryAchievement();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserAchievements::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FOnlineAsyncTaskAccelByte::TriggerDelegates();
	Delegate.ExecuteIfBound(*UserId,bWasSuccessful);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));

}

void FOnlineAsyncTaskAccelByteQueryUserAchievements::QueryAchievement()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Starting query achievement..."));
	
	const THandler<FAccelByteModelsPaginatedUserAchievement> OnQueryAchievementSuccess =
		TDelegateUtils<THandler<FAccelByteModelsPaginatedUserAchievement>>::CreateThreadSafeSelfPtr(this,
			&FOnlineAsyncTaskAccelByteQueryUserAchievements::HandleQueryAchievementSuccess);
	const FErrorHandler OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this,
		&FOnlineAsyncTaskAccelByteQueryUserAchievements::HandleQueryAchievementError);

	ApiClient->Achievement.QueryUserAchievements(
		EAccelByteAchievementListSortBy::NONE,
		OnQueryAchievementSuccess,
		OnError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserAchievements::HandleQueryAchievementSuccess(
	FAccelByteModelsPaginatedUserAchievement const& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""))
	bWasSuccessful = true;
	PaginatedUserAchievements = MakeShared<FAccelByteModelsPaginatedUserAchievement>(Result);

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""))
}

void FOnlineAsyncTaskAccelByteQueryUserAchievements::HandleQueryAchievementError(
	int32 ErrorCode,
	FString const& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), ErrorCode, *ErrorMessage);

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}


void FOnlineAsyncTaskAccelByteQueryUserAchievements::Finalize()
{
	const TSharedPtr<FOnlineAchievementsAccelByte, ESPMode::ThreadSafe> AchievementInterface =
		StaticCastSharedPtr<FOnlineAchievementsAccelByte>(Subsystem->GetAchievementsInterface());

	for(FAccelByteModelsUserAchievement const& UserAchievement : PaginatedUserAchievements->Data)
	{
		TSharedRef<FOnlineAchievement> Achievement = MakeShared<FOnlineAchievement>();
		Achievement->Id = UserAchievement.AchievementCode;
		Achievement->Progress = UserAchievement.LatestValue;

		AchievementInterface->AddUserAchievementToMap(UserId.ToSharedRef(), Achievement);
	}
}



