// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryPlatformSubscription.h"

#include "Interfaces/OnlineEntitlementsInterface.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAccelByteQueryPlatformSubscription"

FOnlineAsyncTaskAccelByteQueryPlatformSubscription::FOnlineAsyncTaskAccelByteQueryPlatformSubscription(
	FOnlineSubsystemAccelByte* const InABSubsystem,
	int32 InLocalUserNum,
	const FOnlineQuerySubscriptionRequestAccelByte& InQueryRequest)
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
{
	LocalUserNum = InLocalUserNum;
	QueryRequest.GroupId = InQueryRequest.GroupId;
	QueryRequest.ProductId = InQueryRequest.ProductId;
	QueryRequest.ActiveOnly = InQueryRequest.ActiveOnly;
	QueryRequest.Offset = InQueryRequest.QueryPaging.Start;
	QueryRequest.Limit = InQueryRequest.QueryPaging.Count;
}

void FOnlineAsyncTaskAccelByteQueryPlatformSubscription::Initialize()
{
	FOnlineAsyncTaskAccelByte::Initialize();

	TRY_PIN_SUBSYSTEM()
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	API_CLIENT_CHECK_GUARD();

	EAccelBytePlatformSync PlatformSync = EAccelBytePlatformSync::OTHER;

	const FString& NativeSubsystemName = SubsystemPin->GetNativePlatformName().ToString();
	if (NativeSubsystemName.Contains("IOS") || NativeSubsystemName.Contains("APPLE"))
	{
		PlatformSync = EAccelBytePlatformSync::APPLE;
	}
	else if (NativeSubsystemName.Contains("GOOGLE"))
	{
		PlatformSync = EAccelBytePlatformSync::GOOGLE;
	}

	if (PlatformSync == EAccelBytePlatformSync::OTHER)
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		ErrorMessage = FString::Printf(TEXT("Current plugin version does not support %s platform yet."), &NativeSubsystemName);
		return;
	}

	auto OnSuccess = TDelegateUtils<THandler<FAccelByteModelsThirdPartyUserSubscriptions>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryPlatformSubscription::HandleQuerySubscriptionSuccess);
	auto OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryPlatformSubscription::HandleQuerySubscriptionError);

	ApiClient->Entitlement.QueryUserSubcriptions(
		PlatformSync,
		QueryRequest,
		OnSuccess,
		OnError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryPlatformSubscription::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	FOnlineAsyncTaskAccelByte::TriggerDelegates();
	const FOnlineEntitlementsAccelBytePtr EntitlementInterface = StaticCastSharedPtr<FOnlineEntitlementsAccelByte>(SubsystemPin->GetEntitlementsInterface());
	if (EntitlementInterface.IsValid())
	{
		EntitlementInterface->TriggerOnQueryPlatformSubscriptionCompleteDelegates(LocalUserNum, bWasSuccessful, *UserId.Get(), SuccessResult.Data, ONLINE_ERROR(bWasSuccessful ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure, FString::Printf(TEXT("%d"), ErrorCode), FText::FromString(ErrorMessage)));
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryPlatformSubscription::HandleQuerySubscriptionSuccess(FAccelByteModelsThirdPartyUserSubscriptions const& Result)
{
	SuccessResult = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
}

void FOnlineAsyncTaskAccelByteQueryPlatformSubscription::HandleQuerySubscriptionError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);

	ErrorCode = Code;
	ErrorMessage = ErrMsg;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE