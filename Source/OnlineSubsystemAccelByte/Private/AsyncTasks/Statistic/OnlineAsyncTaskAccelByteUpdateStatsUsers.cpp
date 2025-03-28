// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUpdateStatsUsers.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineStatsSystemAccelByte"

FOnlineAsyncTaskAccelByteUpdateStatsUsers::FOnlineAsyncTaskAccelByteUpdateStatsUsers(FOnlineSubsystemAccelByte* const InABInterface
	, int32 InLocalUserNum
	, TArray<FOnlineStatsUserUpdatedStats> const& InBulkUpdateMultipleUserStatItems
	, const FOnUpdateMultipleUserStatItemsComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, InLocalUserNum, true)
	, BulkUpdateMultipleUserStatItems(InBulkUpdateMultipleUserStatItems)
	, Delegate(InDelegate)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Construct FOnlineAsyncTaskAccelByteUpdateStatsUsers"));

	BulkUpdateUserStatItemsResult = TArray<FAccelByteModelsUpdateUserStatItemsResponse>{};

	for (auto const& UpdatedUserStat : BulkUpdateMultipleUserStatItems)
	{
		FAccelByteModelsUpdateUserStatItem UpdateUserStatItemWithStatCode;
		AccelByteUserId = FUniqueNetIdAccelByteUser::CastChecked(UpdatedUserStat.Account);
		for (auto const& Stat : UpdatedUserStat.Stats)
		{
			FString Key = Stat.Key;
			FOnlineStatValue Value;
			if (Stat.Value.GetValue().IsNumeric())
			{
				Value = Stat.Value.GetValue();
			}
			FOnlineStatUpdate::EOnlineStatModificationType UpdateStrategy = Stat.Value.GetModificationType();

			UpdateUserStatItemWithStatCode.StatCode = Stat.Key;
			UpdateUserStatItemWithStatCode.Value = FCString::Atof(*Value.ToString());
			UpdateUserStatItemWithStatCode.UpdateStrategy = FOnlineStatisticAccelByte::ConvertUpdateStrategy(UpdateStrategy);
			UpdateUserStatItemWithStatCode.UserId = AccelByteUserId->GetAccelByteId();
			BulkUpdateUserStatItemsRequest.Add(UpdateUserStatItemWithStatCode);
		}

		StatsUsers.Add(UpdatedUserStat.Account);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateStatsUsers::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialized"));

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		ErrorMessage = TEXT("request-failed-bulk-update-users-stats-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to bulk update users stats, identity interface is invalid!"));
		return;
	}

	auto LoginStatus = IdentityInterface->GetLoginStatus(LocalUserNum);
	if (LoginStatus != ELoginStatus::LoggedIn)
	{
		ErrorMessage = TEXT("request-failed-bulk-update-users-stats-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to bulk update users stats, user not logged in!"));
		return;
	}

	OnBulkResetMultipleUserStatItemsValueSuccess = TDelegateUtils<THandler<TArray<FAccelByteModelsUpdateUserStatItemsResponse>>>::CreateThreadSafeSelfPtr(this
		, &FOnlineAsyncTaskAccelByteUpdateStatsUsers::OnBulkUpdateUserStatsSuccess);
	OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this
		, &FOnlineAsyncTaskAccelByteUpdateStatsUsers::OnBulkUpdateUserStatsFailed);

	if (IsRunningDedicatedServer())
	{
		SERVER_API_CLIENT_CHECK_GUARD();
		ServerApiClient->ServerStatistic.BulkUpdateMultipleUserStatItemsValue(BulkUpdateUserStatItemsRequest
			, OnBulkResetMultipleUserStatItemsValueSuccess
			, OnError);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateStatsUsers::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Finalize"));

	const FOnlineStatisticAccelBytePtr StatisticInterface = StaticCastSharedPtr<FOnlineStatisticAccelByte>(SubsystemPin->GetStatsInterface());
	if (StatisticInterface.IsValid())
	{
		for (const auto& UserStatsPair : OnlineUsersStatsPairs)
		{
			StatisticInterface->EmplaceStats(UserStatsPair);
		}
	}
	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		for (const auto& UserStatsPair : OnlineUsersStatsPairs)
		{
			FAccelByteModelsUserStatItemUpdatedPayload UserStatItemUpdatedPayload{};
			UserStatItemUpdatedPayload.UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(UserStatsPair->Account)->GetAccelByteId();
			for (const auto& UserStat : UserStatsPair->Stats)
			{
				UserStatItemUpdatedPayload.StatCodes.Add(UserStat.Key);
			}
			PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsUserStatItemUpdatedPayload>(UserStatItemUpdatedPayload));
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateStatsUsers::TriggerDelegates()
{
	Super::TriggerDelegates();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Trigger Delegates"));
	EOnlineErrorResult Result = bWasSuccessful ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure;

	Delegate.ExecuteIfBound(ONLINE_ERROR(Result, ErrorCode, FText::FromString(ErrorMessage))
		, BulkUpdateUserStatItemsResult);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateStatsUsers::OnBulkUpdateUserStatsSuccess(TArray<FAccelByteModelsUpdateUserStatItemsResponse> const& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("BulkUpdateUserStatItemsValue Success"));
	BulkUpdateUserStatItemsResult = Result;

	for (int i = 0; i < Result.Num(); i++)
	{
		if (!Result[i].Success)
		{
			continue;
		}

		for (auto const& StatUser : StatsUsers)
		{
			AccelByteUserId = FUniqueNetIdAccelByteUser::CastChecked(StatUser);

			if (AccelByteUserId->GetAccelByteId() == Result[i].UserId)
			{
				// Begin Process

				float NewValue = 0.0f;

				TSharedPtr<FJsonValue> NewValueJson = Result[i].Details.JsonObject.Get()->TryGetField(TEXT("currentValue"));
				if (NewValueJson.IsValid() && !NewValueJson->IsNull())
				{
					NewValue = NewValueJson->AsNumber();
				}

				FString Key = Result[i].StatCode;
				FVariantData Value = NewValue;
				Stats.Add(Key, Value);

				OnlineUsersStatsPairs.Add(MakeShared<FOnlineStatsUserStats>(StatUser, Stats));

				break;
			}
		}
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateStatsUsers::OnBulkUpdateUserStatsFailed(int32 Code
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