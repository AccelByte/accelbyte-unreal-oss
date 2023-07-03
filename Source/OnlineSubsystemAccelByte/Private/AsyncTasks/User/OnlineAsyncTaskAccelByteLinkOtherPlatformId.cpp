#include "OnlineAsyncTaskAccelByteLinkOtherPlatformId.h"
#include "OnlineUserInterfaceAccelByte.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineUserSystemAccelByte"

FOnlineAsyncTaskAccelByteLinkOtherPlatformId::FOnlineAsyncTaskAccelByteLinkOtherPlatformId(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FString& InPlatformId, const FString& InTicket)
	: FOnlineAsyncTaskAccelByte(InABSubsystem),
	PlatformId(InPlatformId),
	Ticket(InTicket)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Construct"));

	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));	
}

void FOnlineAsyncTaskAccelByteLinkOtherPlatformId::Initialize()
{
	FOnlineAsyncTaskAccelByte::Initialize();
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialize Linking Account"));
 
	const FVoidHandler& OnLinkOtherPlatformSuccess = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr
		(this, &FOnlineAsyncTaskAccelByteLinkOtherPlatformId::HandleSuccess);
	const FCustomErrorHandler& OnLinkOtherPlatformError = TDelegateUtils<FCustomErrorHandler>::CreateThreadSafeSelfPtr
		(this, &FOnlineAsyncTaskAccelByteLinkOtherPlatformId::HandleError);
	ApiClient->User.LinkOtherPlatformId(PlatformId, Ticket, OnLinkOtherPlatformSuccess, OnLinkOtherPlatformError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));	
}

void FOnlineAsyncTaskAccelByteLinkOtherPlatformId::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("TriggerDelegates"));
	FOnlineAsyncTaskAccelByte::TriggerDelegates();
	
	const FOnlineUserAccelBytePtr UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(Subsystem->GetUserInterface());  
	if (UserInterface.IsValid())
	{
		UserInterface->TriggerOnLinkOtherPlatformIdCompleteDelegates(bWasSuccessful, OnlineError);
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLinkOtherPlatformId::HandleSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleSuccess"));

	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success);
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
} 

void FOnlineAsyncTaskAccelByteLinkOtherPlatformId::HandleError(int32 Code, const FString& Message, const FJsonObject& JsonObject)
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