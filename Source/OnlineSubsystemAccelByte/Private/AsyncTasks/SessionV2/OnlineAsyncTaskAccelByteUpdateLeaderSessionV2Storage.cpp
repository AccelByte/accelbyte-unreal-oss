// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUpdateLeaderSessionV2Storage.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteUpdateLeaderSessionV2Storage"

FOnlineAsyncTaskAccelByteUpdateLeaderSessionV2Storage::FOnlineAsyncTaskAccelByteUpdateLeaderSessionV2Storage(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FNamedOnlineSession* InSessionToUpdate, FJsonObjectWrapper const& InData)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionToUpdate(InSessionToUpdate)
	, Data(InData)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
	AccelByteUserId = UserId.Get()->GetAccelByteId();
}

void FOnlineAsyncTaskAccelByteUpdateLeaderSessionV2Storage::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *AccelByteUserId);

	Super::Initialize();

	OnUpdateStorageSuccessDelegate = TDelegateUtils<THandler<FJsonObjectWrapper>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdateLeaderSessionV2Storage::OnUpdateLeaderStorageSuccess);
	OnUpdateStorageErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdateLeaderSessionV2Storage::OnUpdateLeaderStorageError);

	API_CLIENT_CHECK_GUARD(OnlineError);
	ApiClient->Session.UpdateLeaderStorage(SessionToUpdate->GetSessionIdStr(), Data, OnUpdateStorageSuccessDelegate, OnUpdateStorageErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateLeaderSessionV2Storage::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());

	if (SessionInterface.IsValid())
	{
		SessionInterface->TriggerOnUpdateSessionLeaderStorageCompleteDelegates(SessionToUpdate->SessionName, OnlineError);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
	}
	else
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to TriggerOnUpdateSessionLeaderStorageCompleteDelegates as our session interface is invalid"));
	}
}

void FOnlineAsyncTaskAccelByteUpdateLeaderSessionV2Storage::OnUpdateLeaderStorageSuccess(const FJsonObjectWrapper& ResponseData)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *AccelByteUserId);

	const TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(SessionToUpdate->SessionInfo);
	if(SessionInfo.IsValid())
	{
		SessionInfo->SetSessionLeaderStorage(ResponseData);
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""))
		return;
	}

	ErrorStr = TEXT("update-member-storage-failed-local-session-info-invalid");
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString(), FText::FromString(ErrorStr));
	CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed updating local session leader storage as our session info invalid"))
}

void FOnlineAsyncTaskAccelByteUpdateLeaderSessionV2Storage::OnUpdateLeaderStorageError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *AccelByteUserId);
	ErrorStr = TEXT("update-leader-storage-request-failed");
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString::FromInt(ErrorCode), FText::FromString(ErrorStr));
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to update session leader storage, ErrorCode: %d, ErrorMessage: %s"), ErrorCode, *ErrorMessage);
}

#undef ONLINE_ERROR_NAMESPACE
