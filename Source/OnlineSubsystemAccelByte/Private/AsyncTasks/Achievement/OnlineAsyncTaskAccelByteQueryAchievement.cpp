// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryAchievement.h"

using QueryAchievementSuccessHandler = THandler<FAccelByteModelsPaginatedPublicAchievement>;

FOnlineAsyncTaskAccelByteQueryAchievement::FOnlineAsyncTaskAccelByteQueryAchievement(
	FOnlineSubsystemAccelByte* const Subsystem,
	FUniqueNetId const& InPlayerId,
	FOnQueryAchievementsCompleteDelegate const& InDelegate)
	: FOnlineAsyncTaskAccelByte(Subsystem),
	  Delegate(InDelegate), bWasSuccessful(false)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InPlayerId);
	PaginatedPublicAchievementPtr = MakeShared<FAccelByteModelsPaginatedPublicAchievement>();
}

void FOnlineAsyncTaskAccelByteQueryAchievement::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FOnlineAsyncTaskAccelByte::Initialize();
	QueryAchievement();
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryAchievement::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FOnlineAsyncTask::TriggerDelegates();
	Delegate.ExecuteIfBound(*UserId,bWasSuccessful);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryAchievement::QueryAchievement()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Starting query achievement"));

	const QueryAchievementSuccessHandler OnQueryAchievementSuccess = TDelegateUtils<QueryAchievementSuccessHandler>::CreateThreadSafeSelfPtr(
		this,&FOnlineAsyncTaskAccelByteQueryAchievement::HandleQueryAchievementSuccess);
	const FErrorHandler OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(
		this, &FOnlineAsyncTaskAccelByteQueryAchievement::HandleQueryAchievementError);

	ApiClient->Achievement.QueryAchievements(
		TEXT(""),
		EAccelByteAchievementListSortBy::NONE,
		OnQueryAchievementSuccess,
		OnError);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryAchievement::HandleQueryAchievementSuccess(
	FAccelByteModelsPaginatedPublicAchievement const& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	bWasSuccessful = true;
	PaginatedPublicAchievementPtr = MakeShared<FAccelByteModelsPaginatedPublicAchievement>(Response);

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryAchievement::HandleQueryAchievementError(
	int32 ErrorCode,
	FString const& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), ErrorCode, *ErrorMessage);

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryAchievement::Finalize()
{
	const TSharedPtr<FOnlineAchievementsAccelByte, ESPMode::ThreadSafe> AchievementInterface =
		StaticCastSharedPtr<FOnlineAchievementsAccelByte>(Subsystem->GetAchievementsInterface());
	
	for(auto& Data : PaginatedPublicAchievementPtr->Data)
	{
		const TSharedRef<FOnlineAchievementDesc> Achievement = MakeShared<FOnlineAchievementDesc>();
		Achievement->Title = FText::FromString(Data.Name);
		Achievement->LockedDesc = FText::FromString(Data.Description);
		Achievement->UnlockedDesc = FText::FromString(Data.Description);
		Achievement->bIsHidden = Data.Hidden;
	
		AchievementInterface->AddPublicAchievementToMap(Data.AchievementCode,Achievement);
	}
}


