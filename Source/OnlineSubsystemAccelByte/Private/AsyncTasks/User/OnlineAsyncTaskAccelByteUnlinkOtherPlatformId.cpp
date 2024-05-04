// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUnlinkOtherPlatformId.h"
#include "OnlineUserInterfaceAccelByte.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineUserSystemAccelByte"

FOnlineAsyncTaskAccelByteUnlinkOtherPlatformId::FOnlineAsyncTaskAccelByteUnlinkOtherPlatformId(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FString& InPlatformId)
	: FOnlineAsyncTaskAccelByte(InABSubsystem),
	PlatformId(InPlatformId)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Construct"));

	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));	
}

void FOnlineAsyncTaskAccelByteUnlinkOtherPlatformId::Initialize()
{
	FOnlineAsyncTaskAccelByte::Initialize();
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialize Unlinking Account"));
 
	const FVoidHandler& OnLinkOtherPlatformSuccess = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr
		(this, &FOnlineAsyncTaskAccelByteUnlinkOtherPlatformId::HandleSuccess);
	const FCustomErrorHandler& OnLinkOtherPlatformError = TDelegateUtils<FCustomErrorHandler>::CreateThreadSafeSelfPtr
		(this, &FOnlineAsyncTaskAccelByteUnlinkOtherPlatformId::HandleError);
	API_CLIENT_CHECK_GUARD(OnlineError);
	ApiClient->User.UnlinkAllOtherPlatformId(PlatformId, OnLinkOtherPlatformSuccess, OnLinkOtherPlatformError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));	
}

void FOnlineAsyncTaskAccelByteUnlinkOtherPlatformId::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("TriggerDelegates"));
	FOnlineAsyncTaskAccelByte::TriggerDelegates();
	
	const FOnlineUserAccelBytePtr UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(Subsystem->GetUserInterface());  
	if (UserInterface.IsValid())
	{
		UserInterface->TriggerOnUnlinkOtherPlatformIdCompleteDelegates(bWasSuccessful, OnlineError);
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUnlinkOtherPlatformId::HandleSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleSuccess")); 

	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success);
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
} 

void FOnlineAsyncTaskAccelByteUnlinkOtherPlatformId::HandleError(int32 Code, const FString& Message, const FJsonObject& JsonObject)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleError"));
	
	const FString ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::InvalidRequest);
	OnlineError =  ONLINE_ERROR(EOnlineErrorResult::RequestFailure, ErrorCode, FText::FromString(Message)); 
	const FString ErrorMessage = FString::Printf(TEXT("Error Code: %d; Error Message: %s"), Code, *Message);
	UE_LOG_AB(Warning, TEXT("Failed to unlink account to the AccelByte backend! %s), "), *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE