// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteForcePlatformLinkV3.h"

#include "OnlineUserCacheAccelByte.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineUserSystemAccelByte"

FOnlineAsyncTaskAccelByteForcePlatformLinkV3::FOnlineAsyncTaskAccelByteForcePlatformLinkV3(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FString& InPlatformId, const FString& InTicket)
	: FOnlineAsyncTaskAccelByte(InABSubsystem),
	  PlatformId(InPlatformId),
	  Ticket(InTicket)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Construct"));

	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteForcePlatformLinkV3::Initialize()
{
	FOnlineAsyncTaskAccelByte::Initialize();
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialize Force Linking Platform Account"));

	const FVoidHandler& OnForcePlatformLinkSuccess = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr
		(this, &FOnlineAsyncTaskAccelByteForcePlatformLinkV3::HandleSuccess);
	const FCustomErrorHandler& OnForcePlatformLinkError = TDelegateUtils<FCustomErrorHandler>::CreateThreadSafeSelfPtr
		(this, &FOnlineAsyncTaskAccelByteForcePlatformLinkV3::HandleError);

	API_FULL_CHECK_GUARD(User, OnlineError);
	User->ForcePlatformLinkV3(PlatformId, Ticket, OnForcePlatformLinkSuccess, OnForcePlatformLinkError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteForcePlatformLinkV3::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	const FOnlineUserCacheAccelBytePtr UserCacheInterface = SubsystemPin->GetUserCache();
	auto LinkedUser = UserCacheInterface->GetUser(*UserId.Get());
	if (LinkedUser.IsValid() && LinkedUser->Id.IsValid())
	{
		UserCacheInterface->SetUserDataAsStale(LinkedUser->Id->GetAccelByteId());
	}
}

void FOnlineAsyncTaskAccelByteForcePlatformLinkV3::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("TriggerDelegates"));
	FOnlineAsyncTaskAccelByte::TriggerDelegates();

	const FOnlineUserAccelBytePtr UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(SubsystemPin->GetUserInterface());
	if (UserInterface.IsValid())
	{
		UserInterface->TriggerOnForcePlatformLinkV3CompleteDelegates(bWasSuccessful, OnlineError);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteForcePlatformLinkV3::HandleSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleSuccess"));

	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success);
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteForcePlatformLinkV3::HandleError(int32 Code, const FString& Message, const FJsonObject& JsonObject)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleError"));

	const FString ErrorCode = FString::Printf(TEXT("%d"), Code);
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, ErrorCode, FText::FromString(Message));
	const FString ErrorMessage = FString::Printf(TEXT("Error Code: %d; Error Message: %s"), Code, *Message);
	UE_LOG_AB(Warning, TEXT("Failed to force link platform account! %s"), *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE
