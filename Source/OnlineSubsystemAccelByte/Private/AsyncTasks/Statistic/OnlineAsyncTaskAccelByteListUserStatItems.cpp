// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteListUserStatItems.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineAgreementInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteListUserStatItems::FOnlineAsyncTaskAccelByteListUserStatItems(FOnlineSubsystemAccelByte *const InABInterface
	, int32 InLocalUserNum
	, TArray<FString> const& InStatCodes
	, TArray<FString> const& InTags
	, FString const& InAdditionalKey
	, bool bInAlwaysRequestToService)
	: FOnlineAsyncTaskAccelByte(InABInterface, InLocalUserNum)
	, StatCodes(InStatCodes)
	, Tags(InTags)
	, AdditionalKey(InAdditionalKey)
	, bAlwaysRequestToService(bInAlwaysRequestToService)
{
}

void FOnlineAsyncTaskAccelByteListUserStatItems::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("List user statistic items, UserId: %s")
		, *UserId->ToDebugString());
		
	const IOnlineIdentityPtr IdentityInterface = SubsystemPin->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		ErrorStr = TEXT("list-user-stat-items-failed-identity-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get list user statistic items, Identity Interface is invalid!"));
		return;
	}

	auto LoginStatus = IdentityInterface->GetLoginStatus(LocalUserNum);
	if (LoginStatus != ELoginStatus::LoggedIn)
	{
		ErrorStr = TEXT("list-user-stat-items-failed-not-logged-in");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get list user statistic items, not logged in!"));
		return;
	}

	const FOnlineStatisticAccelBytePtr StatisticInterface = StaticCastSharedPtr<FOnlineStatisticAccelByte>(SubsystemPin->GetStatsInterface());
	if (StatisticInterface.IsValid())
	{
		TArray<TSharedRef<FAccelByteModelsFetchUser>> UsersRef;
		if (StatisticInterface->GetListUserStatItems(LocalUserNum, UsersRef) && !bAlwaysRequestToService)
		{
			TArray<FAccelByteModelsFetchUser> UsersReturn;
			for (const auto& User : UsersRef)
			{
				UsersReturn.Add(User.Get());
			}

			Users = UsersReturn;
			CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
			AB_OSS_ASYNC_TASK_TRACE_END(TEXT("List user statistic items found at user index '%d'!"), LocalUserNum);
		}
		else
		{
			// Create delegates for successfully as well as unsuccessfully querying the user's eligibilities
		 	OnListUserStatItemsSuccessDelegate = TDelegateUtils<THandler<TArray<FAccelByteModelsFetchUser>>>::CreateThreadSafeSelfPtr(this
		 		, &FOnlineAsyncTaskAccelByteListUserStatItems::OnListUserStatItemsSuccess);
			OnListUserStatItemsErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this
				, &FOnlineAsyncTaskAccelByteListUserStatItems::OnListUserStatItemsError);

			// Send off a request to query users, as well as connect our delegates for doing so
			API_FULL_CHECK_GUARD(Statistic, ErrorStr);
			Statistic->ListUserStatItems({}
				, {}
				, TEXT("")
				, OnListUserStatItemsSuccessDelegate
				, OnListUserStatItemsErrorDelegate);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteListUserStatItems::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Finalize"));

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (bWasSuccessful && PredefinedEventInterface.IsValid())
	{
		TSharedPtr<FAccelByteModelsUserStatItemCreatedPayload> UserStatItemCreatedPayload = MakeShared<FAccelByteModelsUserStatItemCreatedPayload>();
		UserStatItemCreatedPayload->UserId = UserId->GetAccelByteId();
		for (const auto& StatItem : Users)
		{
			UserStatItemCreatedPayload->StatCodes.Add(StatItem.StatCode);
		}

		PredefinedEventInterface->SendEvent(LocalUserNum, UserStatItemCreatedPayload.ToSharedRef());
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteListUserStatItems::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s")
		, LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineStatisticAccelBytePtr StatisticInterface = StaticCastSharedPtr<FOnlineStatisticAccelByte>(SubsystemPin->GetStatsInterface());
	if (StatisticInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			TArray<FAccelByteModelsFetchUser> UsersReturn;
			for (const auto& User : Users)
			{
				UsersReturn.Add(User);
			}

			StatisticInterface->TriggerOnListUserStatItemsCompletedDelegates(LocalUserNum
				, true
				, UsersReturn
				, TEXT(""));
		}
		else
		{
			StatisticInterface->TriggerOnListUserStatItemsCompletedDelegates(LocalUserNum
				, false
				, TArray<FAccelByteModelsFetchUser>{}
				, ErrorStr);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteListUserStatItems::OnListUserStatItemsSuccess(TArray<FAccelByteModelsFetchUser> const& Result)
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FOnlineStatisticAccelBytePtr StatisticInterface = StaticCastSharedPtr<FOnlineStatisticAccelByte>(SubsystemPin->GetStatsInterface());
	Users = Result;
	
	if (StatisticInterface.IsValid())
	{
		TArray<TSharedRef<FAccelByteModelsFetchUser>> UsersRef;
		for (const auto& User : Result)
		{
			UsersRef.Add(MakeShared<FAccelByteModelsFetchUser>(User));
		}
		StatisticInterface->UsersMap.Emplace(UserId.ToSharedRef()
			, UsersRef);
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending request to query AccelByte List user statistic items for user '%s'!")
		, *UserId->ToDebugString());
}

void FOnlineAsyncTaskAccelByteListUserStatItems::OnListUserStatItemsError(int32 ErrorCode
	, FString const& ErrorMessage)
{
	ErrorStr = TEXT("request-failed-list-users-stat-items-error");
	UE_LOG_AB(Warning, TEXT("Failed to get list user list statistic items! Error Code: %d; Error Message: %s")
		, ErrorCode
		, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}