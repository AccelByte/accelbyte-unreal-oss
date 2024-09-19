// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteCreateStatsUser.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineError.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineStatsSystemAccelByte"

FOnlineAsyncTaskAccelByteCreateStatsUser::FOnlineAsyncTaskAccelByteCreateStatsUser(FOnlineSubsystemAccelByte *const InABInterface
	, int32 InLocalUserNum
	, FUniqueNetIdRef const& InStatsUser
	, TArray<FString> const& InStatCodes
	, FOnlineStatsCreateStatsComplete const& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, InLocalUserNum, true)
	, StatCodes(InStatCodes)
	, Delegate(InDelegate)
{
	StatsUser = FUniqueNetIdAccelByteUser::CastChecked(InStatsUser);
	CreateStatsResult = TArray<FAccelByteModelsBulkStatItemOperationResult>{};
}

void FOnlineAsyncTaskAccelByteCreateStatsUser::Initialize()
{
	TRY_PIN_SUBSYSTEM()

	Super::Initialize();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	if (!IsRunningDedicatedServer())
	{
		ErrorMessage = TEXT("request-failed-create-user-stats-error-not-ds");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create user stats, not running as Dedicated Server!"));
		return;
	}

	if (!StatsUser.IsValid() || !StatsUser->IsValid())
	{
		ErrorMessage = TEXT("request-failed-create-user-stats-error-user-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create user stats, selected user is invalid!"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		ErrorMessage = TEXT("request-failed-create-user-stats-error-identity-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create user stats, Identity Interface is invalid!"));
		return;
	}

	auto LoginStatus = IdentityInterface->GetLoginStatus(LocalUserNum);
	if (LoginStatus != ELoginStatus::LoggedIn)
	{
		ErrorMessage = TEXT("request-failed-create-user-stats-error-not-logged-in");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create user stats, not logged in!"));
		return;
	}

	OnBulkCreateStatItemsSuccess = TDelegateUtils<THandler<TArray<FAccelByteModelsBulkStatItemOperationResult>>>::CreateThreadSafeSelfPtr(this
		, &FOnlineAsyncTaskAccelByteCreateStatsUser::HandleBulkCreateStatItems);
	OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this
		, &FOnlineAsyncTaskAccelByteCreateStatsUser::HandleAsyncTaskError);
	
	FString AccelByteUserId = StatsUser->GetAccelByteId();
	FServerApiClientPtr ServerApiClient = FMultiRegistry::GetServerApiClient();
	ServerApiClient->ServerStatistic.CreateUserStatItems(AccelByteUserId
		, StatCodes
		, OnBulkCreateStatItemsSuccess
		, OnError);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateStatsUser::Finalize()
{
	TRY_PIN_SUBSYSTEM()

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Finalize"));

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (bWasSuccessful && PredefinedEventInterface.IsValid())
	{
		TSharedPtr<FAccelByteModelsUserStatItemCreatedPayload> UserStatItemCreatedPayload = MakeShared<FAccelByteModelsUserStatItemCreatedPayload>();
		UserStatItemCreatedPayload->UserId = StatsUser->GetAccelByteId();
		for (const auto& StatItem : CreateStatsResult)
		{
			if (StatItem.Success)
			{
				UserStatItemCreatedPayload->StatCodes.Add(StatItem.StatCode);
			}
		}

		PredefinedEventInterface->SendEvent(LocalUserNum, UserStatItemCreatedPayload.ToSharedRef());
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateStatsUser::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	Super::TriggerDelegates();

	EOnlineErrorResult Result = bWasSuccessful ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure;

	Delegate.ExecuteIfBound(ONLINE_ERROR(Result, ErrorCode, FText::FromString(ErrorMessage))
		, CreateStatsResult);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateStatsUser::HandleBulkCreateStatItems(TArray<FAccelByteModelsBulkStatItemOperationResult> const& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	CreateStatsResult = Result;
	
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateStatsUser::HandleAsyncTaskError(int32 Code
	, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s")
		, Code
		, *ErrMsg);

	ErrorCode = FString::Printf(TEXT("%d"), Code);
	ErrorMessage = ErrMsg;

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE