// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUpdateMemberSessionV2Storage.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteUpdateMemberSessionV2Storage"

FOnlineAsyncTaskAccelByteUpdateMemberSessionV2Storage::FOnlineAsyncTaskAccelByteUpdateMemberSessionV2Storage(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FNamedOnlineSession* InSessionToUpdate, FJsonObjectWrapper const& InData)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionToUpdate(InSessionToUpdate)
	, Data(InData)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
	AccelByteUserId = UserId.Get()->GetAccelByteId();
}

void FOnlineAsyncTaskAccelByteUpdateMemberSessionV2Storage::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *AccelByteUserId);

	Super::Initialize();

	OnUpdateStorageSuccessDelegate = TDelegateUtils<THandler<FJsonObjectWrapper>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdateMemberSessionV2Storage::OnUpdateMemberStorageSuccess);
	OnUpdateStorageErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdateMemberSessionV2Storage::OnUpdateMemberStorageError);

	API_CLIENT_CHECK_GUARD(OnlineError);
	ApiClient->Session.UpdateMemberStorage(SessionToUpdate->GetSessionIdStr(), Data, OnUpdateStorageSuccessDelegate, OnUpdateStorageErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateMemberSessionV2Storage::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());

	if (SessionInterface.IsValid())
	{
		SessionInterface->TriggerOnUpdateSessionMemberStorageCompleteDelegates(SessionToUpdate->SessionName, *UserId.Get(), OnlineError);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
	}
	else
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to TriggerOnUpdateSessionMemberStorageCompleteDelegates as our session interface is invalid"));
	}
}

void FOnlineAsyncTaskAccelByteUpdateMemberSessionV2Storage::OnUpdateMemberStorageSuccess(const FJsonObjectWrapper& ResponseData)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *AccelByteUserId);

	const TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(SessionToUpdate->SessionInfo);
	if(SessionInfo.IsValid())
	{
		SessionInfo->SetSessionMemberStorage(UserId.ToSharedRef(), ResponseData);
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""))
		return;
	}

	ErrorText = FText::FromString(TEXT("update-member-storage-failed-local-session-info-invalid"));
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString(), ErrorText);
	CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed updating local session member storage as our session info invalid"))
}

void FOnlineAsyncTaskAccelByteUpdateMemberSessionV2Storage::OnUpdateMemberStorageError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *AccelByteUserId);
	ErrorText = FText::FromString(TEXT("update-member-storage-request-failed"));
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString::FromInt(ErrorCode), ErrorText);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to update session member storage, ErrorCode: %d, ErrorMessage: %s"), ErrorCode, *ErrorMessage);
}

#undef ONLINE_ERROR_NAMESPACE
