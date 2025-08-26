// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteLinkOtherPlatform.h"

#include "OnlineUserCacheAccelByte.h"
#include "OnlineUserInterfaceAccelByte.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineUserSystemAccelByte"

FOnlineAsyncTaskAccelByteLinkOtherPlatform::FOnlineAsyncTaskAccelByteLinkOtherPlatform(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, EAccelBytePlatformType InPlatformType, const FString& InTicket)
	: FOnlineAsyncTaskAccelByte(InABSubsystem),
	PlatformType(InPlatformType),
	Ticket(InTicket)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Construct"));
	
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));	
}

void FOnlineAsyncTaskAccelByteLinkOtherPlatform::Initialize()
{
	FOnlineAsyncTaskAccelByte::Initialize();
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialize Linking Account")); 
	
	const FVoidHandler& OnLinkOtherPlatformSuccess = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr
		(this, &FOnlineAsyncTaskAccelByteLinkOtherPlatform::HandleSuccess);
	const FCustomErrorHandler& OnLinkOtherPlatformError = TDelegateUtils<FCustomErrorHandler>::CreateThreadSafeSelfPtr
		(this, &FOnlineAsyncTaskAccelByteLinkOtherPlatform::HandleError);

	API_FULL_CHECK_GUARD(User, OnlineError);
	User->LinkOtherPlatform(PlatformType, Ticket, OnLinkOtherPlatformSuccess, OnLinkOtherPlatformError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));	
}

void FOnlineAsyncTaskAccelByteLinkOtherPlatform::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	const FOnlineUserCacheAccelBytePtr UserCacheInterface = SubsystemPin->GetUserCache();
	auto LinkedUser = UserCacheInterface->GetUser(*UserId.Get());
	UserCacheInterface->SetUserDataAsStale(LinkedUser->Id->GetAccelByteId());
}

void FOnlineAsyncTaskAccelByteLinkOtherPlatform::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("TriggerDelegates"));
	FOnlineAsyncTaskAccelByte::TriggerDelegates();
	
	const FOnlineUserAccelBytePtr UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(SubsystemPin->GetUserInterface());
	if (UserInterface.IsValid())
	{
		UserInterface->TriggerOnLinkOtherPlatformCompleteDelegates(bWasSuccessful, OnlineError);
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLinkOtherPlatform::HandleSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleSuccess"));

	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success);
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
} 

void FOnlineAsyncTaskAccelByteLinkOtherPlatform::HandleError(int32 Code, const FString& Message, const FJsonObject& JsonObject)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleError")); 	 
	
	const FString ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::InvalidRequest);
	OnlineError =  ONLINE_ERROR(EOnlineErrorResult::RequestFailure, ErrorCode, FText::FromString(Message)); 
	const FString ErrorMessage = FString::Printf(TEXT("Error Code: %d; Error Message: %s"), Code, *Message);
	UE_LOG_AB(Warning, TEXT("Failed to link account to the AccelByte backend! %s), "), *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE