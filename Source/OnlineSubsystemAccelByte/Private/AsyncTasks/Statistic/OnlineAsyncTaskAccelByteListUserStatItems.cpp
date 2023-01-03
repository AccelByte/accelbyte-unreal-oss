// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteListUserStatItems.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineAgreementInterfaceAccelByte.h"

FOnlineAsyncTaskAccelByteListUserStatItems::FOnlineAsyncTaskAccelByteListUserStatItems(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId,
	const TArray<FString>& InStatCodes, const TArray<FString>& InTags, const FString& InAdditionalKey, bool bInAlwaysRequestToService)
	: FOnlineAsyncTaskAccelByte(InABInterface), StatCodes(InStatCodes), Tags(InTags), AdditionalKey(InAdditionalKey), bAlwaysRequestToService(bInAlwaysRequestToService)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteListUserStatItems::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("List user statistic items, UserId: %s"), *UserId->ToDebugString());

	const FOnlineStatisticAccelBytePtr StatisticInterface = StaticCastSharedPtr<FOnlineStatisticAccelByte>(Subsystem->GetStatsInterface());
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
		 	OnListUserStatItemsSuccessDelegate = TDelegateUtils<THandler<TArray<FAccelByteModelsFetchUser>>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteListUserStatItems::OnListUserStatItemsSuccess);
			OnListUserStatItemsErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteListUserStatItems::OnListUserStatItemsError);

			// Send off a request to query users, as well as connect our delegates for doing so
			ApiClient->Statistic.ListUserStatItems({}, {}, "", OnListUserStatItemsSuccessDelegate, OnListUserStatItemsErrorDelegate);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteListUserStatItems::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineStatisticAccelBytePtr StatisticInterface = StaticCastSharedPtr<FOnlineStatisticAccelByte>(Subsystem->GetStatsInterface());
	if (StatisticInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			TArray<FAccelByteModelsFetchUser> UsersReturn;
			for (const auto& User : Users)
			{
				UsersReturn.Add(User);
			}

			StatisticInterface->TriggerOnListUserStatItemsCompletedDelegates(LocalUserNum, true, UsersReturn, TEXT(""));
		}
		else
		{
			StatisticInterface->TriggerOnListUserStatItemsCompletedDelegates(LocalUserNum, false, TArray<FAccelByteModelsFetchUser>{}, ErrorStr);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteListUserStatItems::OnListUserStatItemsSuccess(const TArray<FAccelByteModelsFetchUser>& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FOnlineStatisticAccelBytePtr StatisticInterface = StaticCastSharedPtr<FOnlineStatisticAccelByte>(Subsystem->GetStatsInterface());
	Users = Result;
	
	if (StatisticInterface.IsValid())
	{
		TArray<TSharedRef<FAccelByteModelsFetchUser>> UsersRef;
		for (const auto& User : Result)
		{
			UsersRef.Add(MakeShared<FAccelByteModelsFetchUser>(User));
		}
		StatisticInterface->UsersMap.Emplace(UserId.ToSharedRef(), UsersRef);
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending request to query AccelByte List user statistic items for user '%s'!"), *UserId->ToDebugString());
}

void FOnlineAsyncTaskAccelByteListUserStatItems::OnListUserStatItemsError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorStr = TEXT("request-failed-list-users-stat-items-error");
	UE_LOG_AB(Warning, TEXT("Failed to get list user list statistic items! Error Code: %d; Error Message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}