// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryStatsUsers.h"

FOnlineAsyncTaskAccelByteQueryStatsUsers::FOnlineAsyncTaskAccelByteQueryStatsUsers(FOnlineSubsystemAccelByte* const InABInterface, const TSharedRef<const FUniqueNetId> InLocalUserId,
	const TArray<FUniqueNetIdRef>& InStatsUsers, const TArray<FString>& InStatNames, const FOnlineStatsQueryUsersStatsComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, LocalUserId(InLocalUserId)
	, StatsUsers(InStatsUsers)
	, StatNames(InStatNames)
	, Delegate(InDelegate)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Construct FOnlineAsyncTaskAccelByteQueryStatsUsers"));

	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(LocalUserId);
	for (auto StatsUser : StatsUsers)
	{
		FUniqueNetIdAccelByteUserRef NetId = StaticCastSharedRef<const class FUniqueNetIdAccelByteUser>(StatsUser);
		AccelByteUserIds.Add(NetId->GetAccelByteId());
	}
	CountStatCodesString = 0;
	CountUsers = 0;
	StatCodes = {};
	StatCodesString = {};
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryStatsUsers::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialized"));
	Super::Initialize(); 

	OnGetUserStatItemsSuccess = THandler<FAccelByteModelsUserStatItemPagingSlicedResult>::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryStatsUsers::HandleGetUserStatItems);
	OnError = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryStatsUsers::HandleAsyncTaskError);
	ApiClient->Statistic.GetUserStatItems(AccelByteUserIds[CountUsers], StatNames, {}, OnGetUserStatItemsSuccess, OnError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
} 

void FOnlineAsyncTaskAccelByteQueryStatsUsers::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Trigger Delegates"));
	Super::TriggerDelegates(); 
	Delegate.ExecuteIfBound(OnlineError, OnlineUsersStatsPairs);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryStatsUsers::HandleGetUserStatItems(const FAccelByteModelsUserStatItemPagingSlicedResult& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("GetUserStatItems Success")); 

	for (auto value : Result.Data)
	{
		StatCodesString.Add(value.StatCode);
	}	
	
	OnBulkFetchStatItemsValueSuccess = THandler<TArray<FAccelByteModelsStatItemValueResponse>>::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryStatsUsers::HandleBulkFetchStatItemsValue);
	ApiClient->Statistic.BulkFetchStatItemsValue(StatCodesString[CountStatCodesString], { AccelByteUserIds[CountUsers] }, OnBulkFetchStatItemsValueSuccess, OnError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryStatsUsers::HandleBulkFetchStatItemsValue(const TArray<FAccelByteModelsStatItemValueResponse>& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("BulkFetchStatItemsValue Success"));

	for (FAccelByteModelsStatItemValueResponse User : Result)
	{
		FString Key = User.StatCode;
		FVariantData Value = User.Value;
		StatCodes.Add(Key, Value);
	}
	
	CountStatCodesString++;
	if (CountStatCodesString < StatCodesString.Num())
	{ 
		ApiClient->Statistic.BulkFetchStatItemsValue(StatCodesString[CountStatCodesString], { AccelByteUserIds[CountUsers] }, OnBulkFetchStatItemsValueSuccess, OnError);
	}
	else
	{
		OnlineUsersStatsPairs.Add(MakeShareable(new FOnlineStatsUserStats(StatsUsers[CountUsers], StatCodes)));
		CountUsers++;
		if (CountUsers < AccelByteUserIds.Num())
		{
			CountStatCodesString = 0;
			StatCodesString = {};
			StatCodes = {};
			ApiClient->Statistic.GetUserStatItems(AccelByteUserIds[CountUsers], StatNames, {}, OnGetUserStatItemsSuccess, OnError);
		}
		else
		{
			CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		}
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryStatsUsers::HandleAsyncTaskError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);
 
	OnlineError.SetFromErrorCode(Code);
	OnlineError.SetFromErrorMessage(FText::FromString(ErrMsg));
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}