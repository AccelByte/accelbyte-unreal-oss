// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUpdatePartySessionV2Storage.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteUpdatePartySessionV2Storage"

FOnlineAsyncTaskAccelByteUpdatePartySessionV2Storage::FOnlineAsyncTaskAccelByteUpdatePartySessionV2Storage(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FNamedOnlineSession* InSessionToUpdate, FJsonObjectWrapper const& InData)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionToUpdate(InSessionToUpdate)
	, Data(InData)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
	AccelByteUserId = UserId.Get()->GetAccelByteId();
}

void FOnlineAsyncTaskAccelByteUpdatePartySessionV2Storage::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *AccelByteUserId);

	Super::Initialize();

	OnUpdateStorageSuccessDelegate = TDelegateUtils<THandler<FJsonObjectWrapper>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdatePartySessionV2Storage::OnUpdatePartyStorageSuccess);
	OnUpdateStorageErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdatePartySessionV2Storage::OnUpdatePartyStorageError);

	API_FULL_CHECK_GUARD(Session, OnlineError);
	Session->UpdatePartyStorage(SessionToUpdate->GetSessionIdStr(), Data, OnUpdateStorageSuccessDelegate, OnUpdateStorageErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdatePartySessionV2Storage::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());

	if (SessionInterface.IsValid())
	{
		SessionInterface->TriggerOnUpdatePartySessionStorageCompleteDelegates(SessionToUpdate->SessionName, OnlineError);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
	}
	else
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to TriggerOnUpdatePartySessionStorageCompleteDelegates as our session interface is invalid"));
	}
}

void FOnlineAsyncTaskAccelByteUpdatePartySessionV2Storage::OnUpdatePartyStorageSuccess(const FJsonObjectWrapper& ResponseData)
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

	ErrorStr = TEXT("update-party-storage-failed-local-session-info-invalid");
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString(), FText::FromString(ErrorStr));
	CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed updating local party session storage as our session info invalid"))
}

void FOnlineAsyncTaskAccelByteUpdatePartySessionV2Storage::OnUpdatePartyStorageError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *AccelByteUserId);
	ErrorStr = TEXT("update-party-storage-request-failed");
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString::FromInt(ErrorCode), FText::FromString(ErrorStr));
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to update party session storage, ErrorCode: %d, ErrorMessage: %s"), ErrorCode, *ErrorMessage);
}

#undef ONLINE_ERROR_NAMESPACE
