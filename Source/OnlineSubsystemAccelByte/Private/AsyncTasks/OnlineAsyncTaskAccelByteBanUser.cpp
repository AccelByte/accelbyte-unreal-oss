// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteBanUser.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSessionInterfaceAccelByte.h"

FOnlineAsyncTaskAccelByteBanUser::FOnlineAsyncTaskAccelByteBanUser(FOnlineSubsystemAccelByte* const InABInterface, const FString &InUserId, int32 InActionId, const FString &InMessage)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
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

	const FVoidHandler OnSuccessDelegate = FVoidHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteBanUser::OnSuccess);
    const FErrorHandler OnFailedDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteBanUser::OnFailed);

	TArray<FString> UserIds;
	UserIds.Add(UserId);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
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
