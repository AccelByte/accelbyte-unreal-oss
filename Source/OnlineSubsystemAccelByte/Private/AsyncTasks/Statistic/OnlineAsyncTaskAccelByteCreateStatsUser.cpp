// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteCreateStatsUser.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineError.h"
#include "OnlineIdentityInterfaceAccelByte.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineStatsSystemAccelByte"

FOnlineAsyncTaskAccelByteCreateStatsUser::FOnlineAsyncTaskAccelByteCreateStatsUser(FOnlineSubsystemAccelByte* const InABInterface, const int32 InLocalUserNum,
	const TSharedRef<const FUniqueNetId> InStatsUser, const TArray<FString>& InStatCodes, const FOnlineStatsCreateStatsComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, LocalUserNum(InLocalUserNum)
	, StatsUser(InStatsUser)
	, StatCodes(InStatCodes)
	, Delegate(InDelegate)
{
	AccelByteUserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(StatsUser)->GetAccelByteId();
	CreateStatsResult = TArray<FAccelByteModelsBulkStatItemOperationResult>{};
}

void FOnlineAsyncTaskAccelByteCreateStatsUser::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	Super::Initialize();

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		ErrorMessage = TEXT("request-failed-create-user-stats-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create user stats, identity interface is invalid!"));
		return;
	}

	auto LoginStatus = IdentityInterface->GetLoginStatus(LocalUserNum);
	if (LoginStatus != ELoginStatus::LoggedIn)
	{
		ErrorMessage = TEXT("request-failed-create-user-stats-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create user stats, user not logged in!"));
		return;
	}

	OnBulkCreateStatItemsSuccess = TDelegateUtils<THandler<TArray<FAccelByteModelsBulkStatItemOperationResult>>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteCreateStatsUser::HandleBulkCreateStatItems);
	OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteCreateStatsUser::HandleAsyncTaskError);

	if (IsRunningDedicatedServer())
	{
		FServerApiClientPtr ServerApiClient = FMultiRegistry::GetServerApiClient();
		ServerApiClient->ServerStatistic.CreateUserStatItems(AccelByteUserId,StatCodes, OnBulkCreateStatItemsSuccess, OnError);
	}
	else
	{
		ErrorMessage = TEXT("request-failed-create-user-stats-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create user stats, user not a DS!"));
		return;
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateStatsUser::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	Super::TriggerDelegates();

	EOnlineErrorResult Result = ((bWasSuccessful) ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure);

	Delegate.ExecuteIfBound(ONLINE_ERROR(Result, ErrorCode, FText::FromString(ErrorMessage)), CreateStatsResult);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateStatsUser::HandleBulkCreateStatItems(const TArray<FAccelByteModelsBulkStatItemOperationResult>& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	CreateStatsResult = Result;
	
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateStatsUser::HandleAsyncTaskError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);

	ErrorCode = FString::Printf(TEXT("%d"), Code);
	ErrorMessage = ErrMsg;

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE