// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteBulkUnlockAchievement.h"
#include "OnlineAchievementsInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

FOnlineAsyncTaskAccelByteBulkUnlockAchievement::FOnlineAsyncTaskAccelByteBulkUnlockAchievement(
	FOnlineSubsystemAccelByte* const InSubsystem,
	FUniqueNetId const& InUserID,
	bool bIsServer,
	FAccelByteModelsAchievementBulkUnlockRequest const& InUnlockRequest,
	FOnBulkAchievementUnlockDelegate const& InDelegate)
	: FOnlineAsyncTaskAccelByte(InSubsystem)
	, bIsPerformedByServer(bIsServer)
	, UnlockRequest(InUnlockRequest)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserID);
}

void FOnlineAsyncTaskAccelByteBulkUnlockAchievement::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FOnlineAsyncTaskAccelByte::Initialize();

	auto OnBulkUnlockSuccess = AccelByte::TDelegateUtils <THandler< TArray<FAccelByteModelsAchievementBulkUnlockRespone >>> ::CreateThreadSafeSelfPtr(
		this, &FOnlineAsyncTaskAccelByteBulkUnlockAchievement::HandleBulkUnlockAchievementSuccess);
	auto OnError = AccelByte::TDelegateUtils<AccelByte::FErrorHandler>::CreateThreadSafeSelfPtr(
		this, &FOnlineAsyncTaskAccelByteBulkUnlockAchievement::HandleBulkUnlockAchievementError);

	if (bIsPerformedByServer)
	{
		SERVER_API_CLIENT_CHECK_GUARD();
		if (!UserId.IsValid())
		{
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
			AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
			return;
		}

		FString AccelByteUserID = UserId->GetAccelByteId();
		if (AccelByteUserID.IsEmpty())
		{
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
			AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
			return;
		}

		ServerApiClient->ServerAchievement.BulkUnlockAchievement(AccelByteUserID, UnlockRequest, OnBulkUnlockSuccess, OnError);
	}
	else
	{
		API_FULL_CHECK_GUARD(Achievement);
		Achievement->BulkUnlockAchievement(UnlockRequest, OnBulkUnlockSuccess, OnError);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBulkUnlockAchievement::Finalize() {}

void FOnlineAsyncTaskAccelByteBulkUnlockAchievement::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"),LOG_BOOL_FORMAT(bWasSuccessful));
	
	FOnlineAsyncTask::TriggerDelegates();
	Delegate.ExecuteIfBound(*UserId, BulkUnlockResponse);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBulkUnlockAchievement::HandleBulkUnlockAchievementSuccess(
	TArray<FAccelByteModelsAchievementBulkUnlockRespone> const& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	if (Response.Num() == 0)
	{
		UE_LOG_AB(Log, TEXT("[Warning] Bulk unlock achievement failed, the response is empty!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	BulkUnlockResponse = Response;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBulkUnlockAchievement::HandleBulkUnlockAchievementError(
	int32 ErrorCode,
	FString const& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), ErrorCode, *ErrorMessage);

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
