// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteAuthUser.h"
#include "OnlineAuthInterfaceAccelByte.h"

FOnlineAsyncTaskAccelByteAuthUser::FOnlineAsyncTaskAccelByteAuthUser(FOnlineSubsystemAccelByte* const InABInterface, const FString& InUserId, const FOnAuthUSerCompleted& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
	, UserId(InUserId)
	, bRequestResult(false)
	, Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteAuthUser::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s"), *UserId);

	AB_ASYNC_TASK_DEFINE_SDK_DELEGATES(FOnlineAsyncTaskAccelByteAuthUser, Auth, THandler<FGetUserBansResponse>);
	FRegistry::ServerUser.GetUserBanInfo(UserId, OnAuthSuccessDelegate, OnAuthErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteAuthUser::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	Delegate.ExecuteIfBound(bRequestResult, UserId);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteAuthUser::OnAuthSuccess(const FGetUserBansResponse& InResult)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	if (InResult.Data.Num() == 0)
	{
		bRequestResult = true;
	}
	else
	{
		bRequestResult = false;

#if !UE_BUILD_SHIPPING
		FBanUserResponse ResultData = InResult.Data[0];
		UE_LOG_AB(Warning, TEXT("(%d) Failed to Authenticate for user '%s': Res:UserId{%s}, Ban{%s}, BanId{%s}, Reason{%d}, Namespace{%s}, Enabled{%s}")
			, InResult.Data.Num(), *UserId, *ResultData.UserId, *ResultData.Ban, *ResultData.BanId, ResultData.Reason, *ResultData.Namespace, (ResultData.Enabled ? TEXT("True") : TEXT("False")));
#endif
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending request to query AccelByte Authentication for user '%s' (lists: %d)!"), *UserId, InResult.Data.Num());
}

void FOnlineAsyncTaskAccelByteAuthUser::OnAuthError(int32 InErrorCode, const FString& InErrorMessage)
{
	bRequestResult = false;

	UE_LOG_AB(Warning, TEXT("Failed to Authenticate for user '%s'; Error Code: %d; Error Message: %s"), *UserId, InErrorCode, *InErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
