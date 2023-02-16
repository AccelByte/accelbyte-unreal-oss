// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteBanUser.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSessionInterfaceV1AccelByte.h"

FOnlineAsyncTaskAccelByteBanUser::FOnlineAsyncTaskAccelByteBanUser(FOnlineSubsystemAccelByte* const InABInterface, const FString &InUserId, int32 InActionId, const FString &InMessage)
	: FOnlineAsyncTaskAccelByte(InABInterface, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
	, UserId(InUserId)
	, ActionId(InActionId)
	, Message(InMessage)
{
}

void FOnlineAsyncTaskAccelByteBanUser::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s"), *UserId);

	const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	check(SessionInterface != nullptr);

	const FVoidHandler OnSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteBanUser::OnSuccess);
    const FErrorHandler OnFailedDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteBanUser::OnFailed);

	TArray<FString> UserIds;
	UserIds.Add(UserId);

	// #NOTE (Maxwell): Why is this just failing the task? No longer implemented?
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBanUser::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBanUser::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteBanUser::OnSuccess()
{
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
}

void FOnlineAsyncTaskAccelByteBanUser::OnFailed(int, const FString& ErrorMessage)
{
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
